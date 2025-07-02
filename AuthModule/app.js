const express = require('express');
const jwt = require('jsonwebtoken');
const dotenv = require('dotenv');

dotenv.config();

const app = express();
app.use(express.json());

// For a real application, use a long, complex, and securely stored secret.
const JWT_SECRET = process.env.JWT_SECRET || 'your-default-secret-key';

if (JWT_SECRET === 'your-default-secret-key') {
  console.warn('Warning: Using default JWT secret. Please set a JWT_SECRET environment variable for production.');
}

app.post('/login', (req, res) => {
  // In a real application, you would validate user credentials against a database.
  // For this example, we'll assume authentication is successful
  // and the request body contains the necessary user info.
  const { userId, roles } = req.body;

  if (!userId || !roles) {
    return res.status(400).json({ message: 'userId and roles are required' });
  }

  const payload = {
    userId,
    roles,
  };

  const token = jwt.sign(payload, JWT_SECRET, {
    algorithm: 'HS256',
    expiresIn: '1h'
  });

  res.json({ token });
});

app.post('/verify', (req, res) => {
  console.log('[Auth Server] Received verify request');
  console.log('[Auth Server] Headers:', req.headers);
  console.log('[Auth Server] Body:', req.body);
  
  const { token } = req.body;

  if (!token) {
    console.log('[Auth Server] Error: No token provided');
    return res.status(400).json({ 
      isValid: false,
      message: 'Token is required'
    });
  }

  try {
    const decoded = jwt.verify(token, JWT_SECRET);
    console.log('[Auth Server] Token verified successfully for user:', decoded.userId);
    res.json({
      isValid: true,
      userId: decoded.userId,
      roles: decoded.roles
    });
  } catch (err) {
    console.log('[Auth Server] Token verification failed:', err.message);
    res.status(401).json({
      isValid: false,
      message: 'Invalid token'
    });
  }
});

const port = process.env.PORT || 3000;
if (process.env.NODE_ENV !== 'test') {
  app.listen(port, () => {
    console.log(`Auth server listening on port ${port}`);
  });
}

module.exports = app;