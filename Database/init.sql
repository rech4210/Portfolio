-- init.sql

-- Use the database created in docker-compose
USE mygamedb;

-- Create users table to store login information
CREATE TABLE IF NOT EXISTS users (
    user_id INT AUTO_INCREMENT PRIMARY KEY,
    username VARCHAR(255) NOT NULL UNIQUE,
    password_hash VARCHAR(255) NOT NULL,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
) COMMENT='Stores user account and login information';

-- Create characters table
CREATE TABLE IF NOT EXISTS characters (
    character_id INT AUTO_INCREMENT PRIMARY KEY,
    user_id INT NOT NULL,
    character_name VARCHAR(255) NOT NULL,
    level INT DEFAULT 1,
    exp BIGINT DEFAULT 0,
    json_data JSON,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
    FOREIGN KEY (user_id) REFERENCES users(user_id)
) COMMENT='Stores core data for each player character';

-- Create items table for static item definitions
CREATE TABLE IF NOT EXISTS items (
    item_id INT AUTO_INCREMENT PRIMARY KEY,
    item_name VARCHAR(255) NOT NULL,
    description TEXT,
    item_type VARCHAR(50),
    stats JSON COMMENT 'Stores item stats like attack, defense, etc.',
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
) COMMENT='Static definitions for all items available in the game';

-- Insert some sample data for testing
INSERT INTO users (username, password_hash) VALUES ('testuser', 'fake_password_hash');
INSERT INTO characters (user_id, character_name, level, exp, json_data) VALUES (1, 'Warrior', 10, 5000, '{"items": [101, 201], "skills": ["slash", "bash"]}');
INSERT INTO items (item_name, description, item_type, stats) VALUES ('Health Potion', 'Restores 50 HP.', 'Consumable', '{"hp_restore": 50}');
INSERT INTO items (item_name, description, item_type, stats) VALUES ('Iron Sword', 'A basic sword.', 'Weapon', '{"attack_power": 10}');

-- Create item definitions table
CREATE TABLE IF NOT EXISTS item_definitions (
    item_id INT AUTO_INCREMENT PRIMARY KEY,
    item_name VARCHAR(255) NOT NULL,
    description TEXT,
    stats JSON
);

-- Insert sample data for testing
INSERT INTO item_definitions (item_id, item_name, description, stats) VALUES (101, 'Health Potion', 'Restores 50 HP.', '{"heal": 50}');
INSERT INTO item_definitions (item_id, item_name, description, stats) VALUES (102, 'Mana Potion', 'Restores 30 MP.', '{"mana": 30}');

-- items table: Stores items owned by characters
CREATE TABLE IF NOT EXISTS items (
    item_instance_id BIGSERIAL PRIMARY KEY,
    character_id INT NOT NULL,
    item_id VARCHAR(100) NOT NULL, -- Corresponds to an item ID from game data (e.g., UDataAsset)
    quantity INT NOT NULL DEFAULT 1,
    acquired_at TIMESTAMP WITH TIME ZONE DEFAULT CURRENT_TIMESTAMP,
    CONSTRAINT fk_character
        FOREIGN KEY(character_id)
        REFERENCES characters(character_id)
        ON DELETE CASCADE
);

-- Create indexes for performance
CREATE INDEX IF NOT EXISTS idx_users_username ON users(username);
CREATE INDEX IF NOT EXISTS idx_characters_user_id ON characters(user_id);
CREATE INDEX IF NOT EXISTS idx_items_character_id ON items(character_id); 