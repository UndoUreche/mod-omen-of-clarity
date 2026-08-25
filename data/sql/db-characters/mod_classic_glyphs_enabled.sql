-- add mod table

CREATE TABLE IF NOT EXISTS `mod_classic_glyphs_enabled` (
    `guid` INT UNSIGNED PRIMARY KEY,
    `spec` TINYINT UNSIGNED NOT NULL DEFAULT 0
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;