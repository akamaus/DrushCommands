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
      'nid' => 'number nid or all',
      'pattern' => 'regular expression',
      'replacement' => 'string replacement',
    ),
    'aliases' => array('fn'),
    'bootstrap' => DRUSH_BOOTSTRAP_DRUPAL_FULL
  );
  
  $items['generate-password'] = array(
    'description' => "Generate user password",
    'arguments' => array(
      'user' => 'all or user',
      'user_name' =>'user name',
    ),
    'aliases' => array('genpass'),
    'bootstrap' => DRUSH_BOOTSTRAP_DRUPAL_FULL
  );
  
  $items['rename-user'] = array(
    'description' => "rename user name",
    'arguments' => array(
      'userName' => 'Old user name',
      'newUserName' =>'New user name',
    ),
    'bootstrap' => DRUSH_BOOTSTRAP_DRUPAL_FULL
  );
  
  $items['rename-uid'] = array(
    'description' => "rename user name",
    'arguments' => array(
      'userUid' => 'number uid',
      'newUserName' =>'New user name',
    ),
    'bootstrap' => DRUSH_BOOTSTRAP_DRUPAL_FULL
  );
  
  $items['search-nodes'] = array(
    'description' => "search entry regexps in the body nodes",
    'arguments' => array(
      'pattern' => 'regular expression',
    ),
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
  // Цикл перебора всех нод
  if (drush_drupal_major_version() < 7) {
    echo ("only works in Drupal 7 \n");
    exit(1);
  }
  
  if ($nid=='all'){
  $results = db_query('SELECT nid FROM {node}');
    while ($result=drush_db_fetch_object($results)){
      $node = node_load($result->nid);
        if(ereg($pattern, $node->body['und']['0']['value'])) {
          $str_node=("node/" . $result->nid);
          $aliases = db_query('Select source, alias FROM url_alias WHERE source = :source', array(':source'=>$str_node));       
          $alias=drush_db_fetch_object($aliases);
          echo ($str_node . " - ");
          echo ($alias->alias . "\n");
          $node->body['und']['0']['value'] =ereg_replace($pattern,$replacement,$node->body['und']['0']['value']);
          node_save($node);
        } 
    }
  } else {  
    
    if (!preg_match("/^[0-9]+$/", $nid)) {
      echo "nid must be a number \n";
      exit(2);
    }
    
    $node = node_load($nid);

    if (empty($node)) {
      echo "can't load nid=" . $nid . "\n";
      exit(3);
    }
    
    $node->body['und']['0']['value'] =ereg_replace($pattern,$replacement,$node->body['und']['0']['value']);
    node_save($node);  
    }
}

function drush_bodhy_generate_password($user, $user_name=""){
  
  $edit = array();
  
  //Смена пароля у всех юзеров
  if ($user=='all'){    
    $results=db_query('SELECT uid FROM {users} WHERE uid!=0');
    
      while ( $result = drush_db_fetch_object($results)){
        $account = user_load($result->uid);
        $edit['name'] = $account->name;
        $edit['pass'] = gen_passwd();
        user_save($account, $edit);
        
        echo ("login: " . $account->name . "\n");
        echo ("password: " . $edit['pass'] . "\n");
        echo ("\n");
      }  
      
  } elseif($user=='user' && $user_name!=""){
      //$results=db_query('SELECT uid FROM {users} WHERE name = :name', array(':name'=>$user_name));  
      $results=drush_db_select('users', 'uid', 'name IN (:name)', array(':name'=>$user_name) );
      $edit['name'] = $user_name;
      $edit['pass'] = gen_passwd();    
  
      $account = drush_db_fetch_object($results);
      
        if (empty($account)) {
          echo "can't load account=" . $user_name . "\n";
          exit(4);
        }

      $account = user_load($account->uid);           
      user_save($account, $edit);
  
      echo ("login: " . $account->name . "\n");
      echo ("password: " . $edit['pass'] . "\n");
  } else echo "please specify a user name \n";
}

function drush_bodhy_rename_user($userName="", $newUserName=""){
  
  if ($userName=="" || $newUserName==""){
    echo "please specify the user name and the new user name \n";
    exit(1);
  }
 
  $edit = array();
  
  // Проверяем не повторяется ли новое имя
  // $results=db_query('SELECT uid FROM {users} WHERE name = :name', array(':name'=>$newUserName));
  $results=drush_db_select('users', 'uid', 'name IN (:name)', array(':name'=>$newUserName) );
  $account = drush_db_fetch_object($results);
      
    if (!empty($account)) {
      echo "New name exists \n";
      exit(2);
    }
  
  //$results=db_query('SELECT uid FROM {users} WHERE name = :name', array(':name'=>$userName));
  $results=drush_db_select('users', 'uid', 'name IN (:name)', array(':name'=>$userName) ); 
  $account = drush_db_fetch_object($results);
      
    if (empty($account)) {
      echo "can't load account=" . $userName . "\n";
      exit(3);
    }
  
    if (!preg_match("/^[-_a-zA-Zа-яА-Я0-9]+$/", $newUserName)) {
      echo "New user name is invalid \n";
      exit(4);
    }

  $edit['name'] = $newUserName;
    
  $account = user_load($account->uid);           
  user_save($account, $edit);
}

function drush_bodhy_rename_uid($userUid="", $newUserName=""){

  if ($userUid=="" || $newUserName==""){
    echo "please specify the uid and the new user name \n";
    exit(1);
  }
 
  $edit = array();
  
  // Проверяем не повторяется ли новое имя
  // $results=db_query('SELECT uid FROM {users} WHERE name = :name', array(':name'=>$newUserName));
  $results=drush_db_select('users', 'uid', 'name IN (:name)', array(':name'=>$newUserName));
  
  $account = drush_db_fetch_object($results);

    if (!preg_match("/^[0-9]+$/", $userUid)) {
      echo "uid must be a number \n";
      exit(2);
    }
    
    if (!empty($account)) {
      echo "Name exists \n";
      exit(3);
    }

  $edit['name'] = $newUserName;
  $account = user_load($userUid);

    if (empty($account)) {
      echo "can't load account uid=" . $userUid . "\n";
      exit(4);
    }
    
    if (!preg_match("/^[-_a-zA-Zа-яА-Я0-9]+$/", $newUserName)) {
      echo "New user name is invalid \n";
      exit(5);
    }
    
  user_save($account, $edit);
    
}

function drush_bodhy_search_nodes(string $pattern){  

  if (drush_drupal_major_version() < 7) {
    echo ("only works in Drupal 7 \n");
    exit(1);
  }
  
  if ($pattern=="") {
    echo "please specify the regexp \n";
    exit(2);  
  }
     
  $results = db_query('SELECT nid FROM {node}'); 
  while ($result=drush_db_fetch_object($results)){          
    $node = node_load($result->nid);
      if(ereg($pattern, $node->body['und']['0']['value'])){
        $str_node=("node/" . $result->nid);
        $aliases = db_query('Select source, alias FROM url_alias WHERE source = :source', array(':source'=>$str_node));
        $alias=drush_db_fetch_object($aliases);
        echo ($str_node . " - ");
        echo ($alias->alias . "\n");
      }

  }
  
} 

// Генерация пароля
function gen_passwd() {
 return trim(shell_exec("apg -x 6 -n 1"));
}
