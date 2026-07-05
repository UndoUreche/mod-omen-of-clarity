-- add mod table

CREATE TABLE IF NOT EXISTS `mod_ooc_ff_enabled` (
    `guid` INT UNSIGNED PRIMARY KEY,
    `spec` TINYINT UNSIGNED
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

-- update table with new column

DROP PROCEDURE IF EXISTS `anonymous_block_remove_add_column_if_needed`;

DELIMITER //

CREATE PROCEDURE `anonymous_block_remove_add_column_if_needed`()
BEGIN

	IF NOT EXISTS (
        SELECT 1
        FROM INFORMATION_SCHEMA.COLUMNS
        WHERE TABLE_SCHEMA = DATABASE()
          AND TABLE_NAME = 'mod_ooc_ff_enabled'
          AND COLUMN_NAME = 'spec'
    ) THEN
ALTER TABLE `mod_ooc_ff_enabled`
    ADD COLUMN `spec` TINYINT UNSIGNED;
END IF;
END//

CALL `anonymous_block_remove_add_column_if_needed`();

DROP PROCEDURE IF EXISTS `anonymous_block_remove_add_column_if_needed`;