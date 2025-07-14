-- MyGame 테스트 데이터 삽입 스크립트
-- 개발 및 테스트용 샘플 데이터

-- 테스트 캐릭터 데이터 삽입
INSERT INTO characters (user_id, character_id, character_name, level, exp, json_data) VALUES
('user_001', 'char_001', 'TestWarrior', 5, 1250, '{"Position":{"X":100.0,"Y":200.0,"Z":0.0},"Health":120.0,"Mana":80.0,"Additional":{"Class":"Warrior","LastLogin":"2025-07-15T00:00:00Z"}}'),
('user_002', 'char_002', 'TestMage', 3, 750, '{"Position":{"X":150.0,"Y":250.0,"Z":10.0},"Health":80.0,"Mana":150.0,"Additional":{"Class":"Mage","LastLogin":"2025-07-15T00:00:00Z"}}'),
('user_003', 'char_003', 'TestArcher', 7, 2100, '{"Position":{"X":75.0,"Y":180.0,"Z":5.0},"Health":100.0,"Mana":100.0,"Additional":{"Class":"Archer","LastLogin":"2025-07-15T00:00:00Z"}}');

-- 테스트 인벤토리 데이터 삽입
INSERT INTO inventory (user_id, item_id, quantity, slot_index, item_data) VALUES
-- user_001 (TestWarrior) 인벤토리
('user_001', 'sword_001', 1, 0, '{"ItemType":"Weapon","Durability":100,"EnhancementLevel":2}'),
('user_001', 'health_potion', 5, 1, '{"ItemType":"Consumable","EffectValue":50,"Duration":0}'),
('user_001', 'iron_ore', 10, 2, '{"ItemType":"Material","Quality":"Common","UsedFor":"Crafting"}'),
('user_001', 'warrior_armor', 1, 3, '{"ItemType":"Armor","Defense":25,"Weight":15}'),

-- user_002 (TestMage) 인벤토리
('user_002', 'magic_staff', 1, 0, '{"ItemType":"Weapon","MagicPower":35,"ElementType":"Fire"}'),
('user_002', 'mana_potion', 8, 1, '{"ItemType":"Consumable","EffectValue":30,"Duration":0}'),
('user_002', 'spell_scroll', 3, 2, '{"ItemType":"Consumable","SpellName":"Fireball","ManaCost":15}'),
('user_002', 'mage_robe', 1, 3, '{"ItemType":"Armor","Defense":10,"ManaBonus":20}'),

-- user_003 (TestArcher) 인벤토리
('user_003', 'hunting_bow', 1, 0, '{"ItemType":"Weapon","Damage":28,"Range":150}'),
('user_003', 'arrow_bundle', 50, 1, '{"ItemType":"Ammunition","ArrowType":"Steel","Penetration":15}'),
('user_003', 'leather_armor', 1, 2, '{"ItemType":"Armor","Defense":18,"Agility":5}'),
('user_003', 'stamina_potion', 3, 3, '{"ItemType":"Consumable","EffectValue":40,"Duration":60}');

-- 테스트 스킬 데이터 삽입 (향후 확장용)
-- skill_data는 AssetManager에서 관리되므로, JSON 으로 딱히 관리 안해도 될듯
INSERT INTO skills (user_id, slot_id, skill_id, slot_index, last_used_time, remaining_cooldown, is_active, skill_data) VALUES
('0', UUID(), 100, 0, '2025-07-15 00:00:00', 0.0, TRUE, '{"SkillName":"PowerStrike","Damage":150,"ManaCost":20,"CooldownSeconds":5}'),
('0', UUID(), 101, 1, '2025-07-15 00:00:00', 0.0, TRUE, '{"SkillName":"DefenseStance","DefenseBonus":50,"Duration":30,"ManaCost":15}'),
('0', UUID(), 102, 2, '2025-07-15 00:00:00', 0.0, TRUE, '{"SkillName":"Fireball","Damage":120,"ManaCost":25,"Range":100}'),
('0', UUID(), 103, 3, '2025-07-15 00:00:00', 0.0, TRUE, '{"SkillName":"Teleport","Distance":50,"ManaCost":30,"CooldownSeconds":10}'),
('0', UUID(), 104, 4, '2025-07-15 00:00:00', 0.0, TRUE, '{"SkillName":"PrecisionShot","Damage":200,"Accuracy":95,"ManaCost":10}'),
('0', UUID(), 105, 5, '2025-07-15 00:00:00', 0.0, TRUE, '{"SkillName":"QuickReload","ReloadSpeed":200,"Duration":15,"ManaCost":5}');

('1', UUID(), 100, 0, '2025-07-15 00:00:00', 0.0, TRUE, '{"SkillName":"PowerStrike","Damage":150,"ManaCost":20,"CooldownSeconds":5}'),
('1', UUID(), 101, 1, '2025-07-15 00:00:00', 0.0, TRUE, '{"SkillName":"DefenseStance","DefenseBonus":50,"Duration":30,"ManaCost":15}'),
('1', UUID(), 102, 2, '2025-07-15 00:00:00', 0.0, TRUE, '{"SkillName":"Fireball","Damage":120,"ManaCost":25,"Range":100}'),
('1', UUID(), 103, 3, '2025-07-15 00:00:00', 0.0, TRUE, '{"SkillName":"Teleport","Distance":50,"ManaCost":30,"CooldownSeconds":10}'),
('1', UUID(), 104, 4, '2025-07-15 00:00:00', 0.0, TRUE, '{"SkillName":"PrecisionShot","Damage":200,"Accuracy":95,"ManaCost":10}'),
('1', UUID(), 105, 5, '2025-07-15 00:00:00', 0.0, TRUE, '{"SkillName":"QuickReload","ReloadSpeed":200,"Duration":15,"ManaCost":5}');

-- 테스트 장비 데이터 삽입 (향후 확장용)
INSERT INTO equipment (user_id, item_id, slot_index, slot_type, is_equipped, enhancement_level) VALUES
('user_001', 1001, 0, 'MainHand', TRUE, '{"Level":2,"Effects":["Damage+10","Crit+5"]}'),
('user_001', 1002, 1, 'Chest', TRUE, '{"Level":1,"Effects":["Defense+15"]}'),
('user_002', 2001, 0, 'MainHand', TRUE, '{"Level":3,"Effects":["MagicPower+20","Mana+15"]}'),
('user_002', 2002, 1, 'Chest', TRUE, '{"Level":1,"Effects":["Defense+8","ManaRegen+5"]}'),
('user_003', 3001, 0, 'MainHand', TRUE, '{"Level":1,"Effects":["Range+25","Accuracy+10"]}'),
('user_003', 3002, 1, 'Chest', TRUE, '{"Level":2,"Effects":["Defense+12","Agility+8"]}');

-- 테스트 상점 데이터 삽입 (향후 확장용)
INSERT INTO shops (shop_id, shop_name, shop_description, is_open, area_id, shop_location_x, shop_location_y, shop_location_z, last_restock_time, global_price_modifier, shop_owner_name) VALUES
(1, 'Warriors Arsenal', 'The finest weapons and armor for brave warriors', TRUE, 1, 500.0, 300.0, 0.0, '2025-07-15 00:00:00', 1.0, 'Blacksmith Gareth'),
(2, 'Mystical Emporium', 'Magical items and arcane knowledge', TRUE, 1, 600.0, 350.0, 0.0, '2025-07-15 00:00:00', 1.1, 'Wizard Alaric'),
(3, 'Rangers Supply', 'Everything an archer needs for the hunt', TRUE, 2, 450.0, 250.0, 0.0, '2025-07-15 00:00:00', 0.95, 'Ranger Sylvia');

-- 테스트 상점 아이템 데이터 삽입
INSERT INTO shop_items (shop_id, item_id, item_name, item_description, price, stock, is_available, category, max_stock, restock_interval_hours) VALUES
-- Warriors Arsenal (shop_id: 1)
(1, 1001, 'Iron Sword', 'A reliable iron sword for any warrior', 150.0, 5, TRUE, 'Weapons', 10, 24.0),
(1, 1002, 'Steel Shield', 'Strong steel shield for defense', 120.0, 3, TRUE, 'Armor', 5, 48.0),
(1, 1003, 'Health Potion', 'Restores 50 HP instantly', 25.0, 20, TRUE, 'Consumables', 50, 6.0),
(1, 1004, 'Iron Ore', 'Raw material for crafting', 10.0, 100, TRUE, 'Materials', 200, 12.0),

-- Mystical Emporium (shop_id: 2)
(2, 2001, 'Fire Staff', 'Staff imbued with fire magic', 300.0, 2, TRUE, 'Weapons', 3, 72.0),
(2, 2002, 'Mana Crystal', 'Increases maximum mana', 200.0, 4, TRUE, 'Accessories', 6, 48.0),
(2, 2003, 'Mana Potion', 'Restores 30 MP instantly', 30.0, 15, TRUE, 'Consumables', 40, 6.0),
(2, 2004, 'Spell Scroll', 'Single-use magic spell', 75.0, 8, TRUE, 'Consumables', 20, 24.0),

-- Rangers Supply (shop_id: 3)
(3, 3001, 'Hunting Bow', 'Excellent bow for hunting', 180.0, 4, TRUE, 'Weapons', 6, 36.0),
(3, 3002, 'Steel Arrows', 'High-quality steel arrows', 15.0, 50, TRUE, 'Ammunition', 100, 12.0),
(3, 3003, 'Leather Armor', 'Light but protective leather armor', 100.0, 6, TRUE, 'Armor', 10, 48.0),
(3, 3004, 'Stamina Potion', 'Restores energy and reduces fatigue', 20.0, 25, TRUE, 'Consumables', 50, 8.0);

SELECT 'Test data insertion completed successfully!' as status;
