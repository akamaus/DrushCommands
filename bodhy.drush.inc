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
  
  $items['filter-nodes'] = array(
    'description' => "filters all nodes bodies replacing strings matching a given regexp",
    'arguments' => array(
      'nid' => 'number nid',
      'pattern' => 'regular expression',
      'replacement' => 'string replacement',
    ),
    'aliases' => array('fn'),
    'bootstrap' => DRUSH_BOOTSTRAP_DRUPAL_FULL
  );
  
  $items['generate-password'] = array(
    'description' => "fff",
    'arguments' => array(
      'user' => 'all or user',
      'user_name' =>'user name',
    ),
    'aliases' => array('genpass'),
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

function drush_bodhy_filter_nodes(string $nid, string $pattern, string $replacement) { 
  // Цикл перепобра всех нод
  if ($nid=='all'){
  $results = db_query('SELECT nid FROM {node}');
    while ($result=$results->fetchObject()){
      $node = node_load($result->nid);
      $node->body['und']['0']['value'] =ereg_replace($pattern,$replacement,$node->body['und']['0']['value']);
      node_save($node);
    }
  } else {  
    
    if (!preg_match("/^[0-9]+$/", $nid)) {
      echo "nid must be a number \n";
      exit(1);
    }
    
    $node = node_load($nid);

    if (empty($node)) {
      echo "can't load nid=" . $nid . "\n";
      exit(2);
    }
    
    $node->body['und']['0']['value'] =ereg_replace($pattern,$replacement,$node->body['und']['0']['value']);
    node_save($node);  
    }
}

function drush_bodhy_generate_password($user, $user_name=""){
  
  $edit = array();
  if ($user=='all'){    
    $results=db_query('SELECT uid FROM {users} WHERE uid!=0');
    
      while ( $result = $results->fetchObject()){
        $account = user_load($result->uid);
        $edit['name'] = $account->name;
        $edit['pass'] = gen_passwd();
        user_save($account, $edit);
        
        echo ("login: " . $account->name . "\n");
        echo ("password: " . $edit['pass'] . "\n");
        echo ("\n");
      }  
  
  } elseif($user=='user' && $user_name!=""){
      $results=db_query('SELECT uid FROM {users} WHERE name = :name', array(':name'=>$user_name));  
      $edit['name'] = $user_name;
      $edit['pass'] = gen_passwd();    
  
      $account = $results->fetchObject();
      
        if (empty($account)) {
          echo "can't load account=" . $user_name . "\n";
          exit(1);
        }

      $account = user_load($account->uid);
              
      user_save($account, $edit);
  
      echo ("login: " . $account->name . "\n");
      echo ("password: " . $edit['pass'] . "\n");
  } else echo "please specify a user name";
}

function gen_passwd() {
 return trim(shell_exec("apg -x 6 -n 1"));
}