ALTER TABLE `access_requirement` ADD COLUMN `itemlevel_min` INT(10) UNSIGNED DEFAULT '0' NOT NULL AFTER `completed_achievement`;
ALTER TABLE `access_requirement` ADD COLUMN `itemlevel_max` INT(10) UNSIGNED DEFAULT '0' NOT NULL AFTER `itemlevel_min`;