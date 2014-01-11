<?php

/**
 * Implementation of hook_drush_command().
 */
function nyccamp_migrate_drush_command() {
  return array(
    'nyccamp-migrate-user-fields' => array(
      'description' => 'Migrates user field data',
      'bootstrap' => DRUSH_BOOTSTRAP_DRUPAL_FULL,
      'aliases' => array('migrate-users'),
      'options' => array(
        'limit' => 'How many users to update',
        // 'source' => 'The fieldname source.',
        // 'dest' => 'The fieldname destination.', // These should actually be arguments
      ),
      'examples' => array(
        // 'drush migrate-users --source="field_profile_org_bio" dest="field_bio"' => '',
      ),
    ),
  );
}


function drush_nyccamp_migrate_user_fields() {
  $limit = drush_get_option('limit', 5000);

  // Example:
  // field_profile_org_bio
  // field_bio

  // Has the following tables:
  // field_data_field_profile_org_bio
  // field_revision_field_profile_org_bio

  // field_data_field_bio
  // field_revision_field_bio

  // Other tables to consider:
  // field_config
  // field_config_instance

  // First Step to moving data out of profile_org_bio into field_bio:
  // Get all of the user ids with a value in profile_org_bio:
  $query = new EntityFieldQuery();
  $result = $query->entityCondition('entity_type', 'user')
    ->entityCondition('bundle', 'user')
    ->fieldCondition('field_profile_org_bio', 'value', '', '!=')
    ->range(0, $limit)->execute();

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
  field_delete_field('field_profile_org_bio');

  drush_log('Marked field_profile_org_bio for deletion', 'success');
  field_purge_batch(100);
  drush_log('Purged deleted fields.', 'success');

}
