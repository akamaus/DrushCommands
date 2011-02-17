<?php

function proc_rows($query, $accum) {
  $ret = array();
  $result = db_query($query);

  while ($row = db_fetch_array($result)) {
    $accum($ret, $row);
  }
  return $ret;
}

$node_urls = proc_rows('SELECT nid FROM {node}',
                       function(&$ar, $row) { $ar[] = drupal_lookup_path('alias', 'node/' . reset($row)); });


$bad_links = proc_rows('SELECT lid,url FROM {linkchecker_links} where code=404',
                       function(&$ar, $row) { $ar[$row['lid']] = $row['url']; });

print_r($node_urls);
print "\n";

print_r ($bad_links);
print "\n";