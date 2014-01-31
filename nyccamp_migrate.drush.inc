<?php

/**
 * Implementation of hook_drush_command().
 */
function nyccamp_migrate_drush_command() {
  $items = array(
    'copy-bio' => array(
      'description' => 'Copies bio user field data',
      'bootstrap' => DRUSH_BOOTSTRAP_DRUPAL_FULL,
      'options' => array(
        'src' => 'The fieldname source.',
        'dst' => 'The fieldname destination.',
        'limit' => 'How many users to update.',
      ),
      'examples' => array(
        'drush copy-bio --src="field_profile_bio" --dst="field_bio" ' => 'Copies data from field_profile_bio to field_bio',
      ),
    ),
  );

  return $items;
}

/**
 * Implements drush_hook_COMMAND_validate()
 */
function drush_nyccamp_migrate_copy_bio_validate() {
  $okay = TRUE;
  
  $src = trim(drush_get_option('src', ''));
  if(empty($src)) {
    $okay = drush_set_error('INVALID SRC', dt('--src option is missing or empty'));
  }
  
  $dst = trim(drush_get_option('dst', ''));
  if(empty($dst)) {
    $okay = drush_set_error('INVALID DST', dt('--dst option is missing or empty'));
  }
  
  $limit = intval(drush_get_option('limit', 5000));
  if($limit <= 0 ) {
    $okay = drush_set_error('INVALID LIMIT', dt('--limit option must be positive integer'));
  }
  
  return $okay;
}

/**
 * Implements drush_hook_COMMAND()
 */
function drush_nyccamp_migrate_copy_bio() {
  $src = trim(drush_get_option('src', ''));
  $dst = trim(drush_get_option('dst', ''));
  $limit = intval(drush_get_option('limit', 5000));

  $query = new EntityFieldQuery();
  
  return;  // Everything below this return needs to be reviewed and tested. EF

  // Example:
  // field_profile_org_bio
  // field_bio

  // Has the following tables:
  // field_data_field_profile_org_bio    DB ROWS  1
  // field_revision_field_profile_org_bio    DB ROWS  0

  // field_data_field_bio    DB ROWS  63
  // field_revision_field_bio    DB ROWS  63

  // Other tables to consider:
  // field_config
  // field_config_instance

  // First Step to moving data out of profile_org_bio into field_bio:
  // Get all of the user ids with a value in profile_org_bio:
  $result = $query->entityCondition('entity_type', 'user')
      ->entityCondition('bundle', 'user')
      ->fieldCondition('field_profile_org_bio', 'value', '', '!=')
      ->range(0, $limit)
      ->execute();

  // If we have results
  if (isset($result['user'])) {
    // Load all the user objects
    $uids = array_keys($result['user']);
    $users = user_load_multiple($uids);

    $num_changed = 0;

    // Loop thru these user objects and move the data from profile_org_bio to field_bio
    foreach($users as $user) {
      // Make sure that there isn't a conflict -- is the bio field already populated?
      if (empty($user->field_bio)) {
        // Set the value of field_bio to be what's stored in profile_org_bio
        // ....

        // save the user object
        user_save($user);
        drush_log(sprintf('Updated bio for uid [%d]', $user->uid), 'success');
        $num_changed++;
      }
      else {
        // drush_log() that can't overwrite,
      }
    }

    drush_log(sprintf('Updated %d users', $num_changed), 'success');
  }

  // Delete the profile_org_bio field
  //*** I'd suggest making the delete field a separate command, just to be conservative. EF
  field_delete_field('field_profile_org_bio');

  drush_log('Marked field_profile_org_bio for deletion', 'success');
  field_purge_batch(100);
  drush_log('Purged deleted fields.', 'success');
}
