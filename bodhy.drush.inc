<?php

function bodhy_drush_command() {
  $items = array();

  $items['update-node'] = array(
    'description' => "Rewrites a node body",
    'arguments' => array(
      'nid' => 'nid of node to update',
      'filename' => 'file with contents',
    ),
    'options' => array(),
    'examples' => array(
      'drush update-node 500 test.txt',
    ),
    'aliases' => array('un'),
    'bootstrap' => DRUSH_BOOTSTRAP_DRUPAL_FULL //DRUSH_BOOTSTRAP_DRUSH, // No bootstrap at all.
  );

  $items['enable_security_email_notifications'] = array(
    'description' => 'Enables email notifications to given address',
    'arguments' => array('email' => 'email to receive notifications'),
    'aliases' => array('eesn'),
    'bootstrap' => DRUSH_BOOTSTRAP_DRUPAL_FULL
  );

  return $items;
}

function bodhy_drush_help() {
}

function drush_bodhy_update_node($nid, $filename) {
  $node = node_load($nid);

  if (empty($node)) {
    echo "can't load nid=" . $nid . "\n";
    exit(1);
  }

  $content = file_get_contents($filename);

  if (empty($content)) {
    echo "can't read file data\n";
    exit(2);
  }

  $node->body = $content;

  node_save($node);
}

function drush_bodhy_enable_security_email_notifications($email) {
  variable_set('update_check_disabled', 0);
  variable_set('update_check_frequency', 1);
  variable_set('update_notification_threshold', 'security');

  if (preg_match("/^[a-z0-9-.]+@[a-z0-9-.]+$/", $email)) {
    variable_set("update_notify_emails", array(0 => $email));
    drush_log("email notifications enabled", 'ok');
  } else
    drush_log("can't parse email", 'error');
}
