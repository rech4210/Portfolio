const express = require('express');
const jwt = require('jsonwebtoken');
const dotenv = require('dotenv');
const bcrypt = require('bcrypt');
const mysql = require('mysql2/promise');
const { v4: uuidv4 } = require('uuid');

dotenv.config();

const app = express();
app.use(express.json());

const JWT_SECRET = process.env.JWT_SECRET || 'your-default-secret-key';
const SALT_ROUNDS = 10;

// --- Database Connection Pool ---
// UE5 AuthComponent 스키마와 호환되는 데이터베이스 연결
const dbPool = mysql.createPool({
    host: process.env.DB_HOST || '127.0.0.1',
    port: process.env.DB_PORT || 3307,
    user: process.env.DB_USER || 'Regacy',
    password: process.env.DB_PASSWORD || 'GGwa1234',
    database: process.env.DB_NAME || 'MyGameDB',
    waitForConnections: true,
    connectionLimit: 10,
    queueLimit: 0,
    dateStrings: true, 
    timezone: '+00:00'
});

// --- Registration Endpoint (UE5 AuthComponent Compatible) ---
app.post('/register', async (req, res) => {
    const { username, password } = req.body;
    const clientIp = req.ip || req.connection.remoteAddress || '127.0.0.1';

    if (!username || !password) {
        return res.status(400).json({ 
            success: false,
            message: 'Username and password are required' 
        });
    }

    // UE5 AuthComponent validation rules
    if (username.length < 3 || username.length > 30) {
        return res.status(400).json({ 
            success: false,
            message: 'Username must be between 3 and 30 characters' 
        });
    }

    if (password.length < 8) {
        return res.status(400).json({ 
            success: false,
            message: 'Password must be at least 8 characters long' 
        });
    }

    let connection;
    try {
        const hashedPassword = await bcrypt.hash(password, SALT_ROUNDS);
        const userId = uuidv4(); // Generate UUID for user_id to match UE5 AuthComponent

        connection = await dbPool.getConnection();
        await connection.beginTransaction();

        // 1. Check if username already exists
        const checkSql = 'SELECT user_id FROM users WHERE username = ?';
        const [existingUsers] = await connection.execute(checkSql, [username]);
        
        if (existingUsers.length > 0) {
            await connection.rollback();
            return res.status(409).json({ 
                success: false,
                message: 'Username already exists' 
            });
        }

        // 2. Insert user using UE5 AuthComponent schema
        const userSql = `
            INSERT INTO users (user_id, username, password_hash, created_at, last_login_at, is_locked, lock_expires_at, is_deleted, deleted_at) 
            VALUES (?, ?, ?, NOW(3), NULL, 0, NULL, 0, NULL)
        `;
        await connection.execute(userSql, [userId, username, hashedPassword]);

        // 3. Insert audit log using UE5 schema
        const logSql = `
            INSERT INTO user_audit_logs (user_id, action, detail, created_at) 
            VALUES (?, ?, ?, NOW(3))
        `;
        const details = JSON.stringify({ 
            username: username,
            ip: clientIp, 
            userAgent: req.headers['user-agent'] || 'unknown'
        });
        await connection.execute(logSql, [userId, 'registration', details]);

        await connection.commit();

        res.status(201).json({ 
            success: true,
            message: 'User registered successfully', 
            userId: userId 
        });

    } catch (error) {
        if (connection) await connection.rollback();

        if (error.code === 'ER_DUP_ENTRY') {
            res.status(409).json({ 
                success: false,
                message: 'Username already exists' 
            });
        } else {
            console.error('Registration error:', error);
            res.status(500).json({ 
                success: false,
                message: 'Internal server error' 
            });
        }
    } finally {
        if (connection) connection.release();
    }
});

// --- Login Endpoint (UE5 AuthComponent Compatible) ---
app.post('/login', async (req, res) => {
    const { username, password } = req.body;
    const clientIp = req.ip || req.connection.remoteAddress || '127.0.0.1';

    if (!username || !password) {
        return res.status(400).json({ 
            success: false,
            message: 'Username and password are required' 
        });
    }

    let connection;
    try {
        connection = await dbPool.getConnection();

        // Get user data using UE5 AuthComponent schema
        const userSql = `
            SELECT user_id, username, password_hash, is_locked, lock_expires_at, is_deleted 
            FROM users 
            WHERE username = ? AND is_deleted = 0
        `;
        const [rows] = await connection.execute(userSql, [username]);

        if (rows.length === 0) {
            // Log failed attempt for non-existent user
            const logSql = `
                INSERT INTO user_audit_logs (user_id, action, detail, created_at) 
                VALUES (?, ?, ?, NOW(3))
            `;
            const details = JSON.stringify({ 
                attempted_username: username,
                ip: clientIp,
                userAgent: req.headers['user-agent'] || 'unknown'
            });
            await connection.execute(logSql, [null, 'login_failed_user_not_found', details]);

            return res.status(401).json({ 
                success: false,
                message: 'Invalid username or password' 
            });
        }

        const user = rows[0];

        // Check if account is locked using UE5 AuthComponent logic
        if (user.is_locked) {
            const now = new Date();
            const lockExpires = user.lock_expires_at ? new Date(user.lock_expires_at) : null;
            
            // If no expiration date or lock hasn't expired yet
            if (!lockExpires || now < lockExpires) {
                const logSql = `
                    INSERT INTO user_audit_logs (user_id, action, detail, created_at) 
                    VALUES (?, ?, ?, NOW(3))
                `;
                const details = JSON.stringify({ 
                    ip: clientIp,
                    lockExpires: lockExpires ? lockExpires.toISOString() : 'permanent',
                    userAgent: req.headers['user-agent'] || 'unknown'
                });
                await connection.execute(logSql, [user.user_id, 'login_failed_locked', details]);

                return res.status(403).json({ 
                    success: false,
                    message: 'Account is locked' 
                });
            }
        }

        // Verify password
        const isPasswordMatch = await bcrypt.compare(password, user.password_hash);

        if (!isPasswordMatch) {
            // Log failed password attempt
            const logSql = `
                INSERT INTO user_audit_logs (user_id, action, detail, created_at) 
                VALUES (?, ?, ?, NOW(3))
            `;
            const details = JSON.stringify({ 
                ip: clientIp,
                userAgent: req.headers['user-agent'] || 'unknown'
            });
            await connection.execute(logSql, [user.user_id, 'login_failed_password', details]);

            return res.status(401).json({ 
                success: false,
                message: 'Invalid username or password' 
            });
        }

        // Successful login - start transaction for updates
        await connection.beginTransaction();

        // 1. Update last login time and unlock if lock expired
        let updateSql = `UPDATE users SET last_login_at = NOW(3)`;
        const updateParams = [];
        
        // If account was locked but lock expired, automatically unlock
        if (user.is_locked && user.lock_expires_at) {
            const now = new Date();
            const lockExpires = new Date(user.lock_expires_at);
            if (now >= lockExpires) {
                updateSql += `, is_locked = 0, lock_expires_at = NULL`;
            }
        }
        
        updateSql += ` WHERE user_id = ?`;
        updateParams.push(user.user_id);
        
        await connection.execute(updateSql, updateParams);

        // 2. Insert success audit log
        const logSql = `
            INSERT INTO user_audit_logs (user_id, action, detail, created_at) 
            VALUES (?, ?, ?, NOW(3))
        `;
        const details = JSON.stringify({ 
            ip: clientIp,
            userAgent: req.headers['user-agent'] || 'unknown'
        });
        await connection.execute(logSql, [user.user_id, 'login_success', details]);
        
        await connection.commit();

        // Generate JWT
        const payload = { 
            userId: user.user_id, 
            username: user.username
        };
        const token = jwt.sign(payload, JWT_SECRET, { expiresIn: '1h' });

        res.json({ 
            success: true,
            token: token,
            user: {
                userId: user.user_id,
                username: user.username
            }
        });

    } catch (error) {
        if (connection) await connection.rollback();
        console.error('Login error:', error);
        res.status(500).json({ 
            success: false,
            message: 'Internal server error' 
        });
    } finally {
        if (connection) connection.release();
    }
});

// --- Token Verification Endpoint ---
app.post('/verify', (req, res) => {
    const { token } = req.body;

    if (!token) {
        return res.status(400).json({ 
            success: false,
            message: 'Token is required' 
        });
    }

    jwt.verify(token, JWT_SECRET, (err, decoded) => {
        if (err) {
            return res.status(401).json({ 
                success: false,
                message: 'Invalid or expired token' 
            });
        }
        
        res.json({ 
            success: true,
            decoded: decoded 
        });
    });
});

// --- Health Check Endpoint ---
app.get('/health', (req, res) => {
    res.json({ 
        success: true,
        message: 'Auth server is running (UE5 Compatible)',
        timestamp: new Date().toISOString()
    });
});

// --- Error Handling Middleware ---
app.use((err, req, res, next) => {
    console.error('Unhandled error:', err);
    res.status(500).json({ 
        success: false,
        message: 'Internal server error' 
    });
});

const PORT = process.env.PORT || 3000;
app.listen(PORT, () => {
    console.log(`Auth server (UE5 Compatible) running on port ${PORT}`);
});
