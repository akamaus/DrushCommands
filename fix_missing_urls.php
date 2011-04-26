<?php

function proc_rows($query, $accum) {
  $ret = array();
  $result = db_query($query);

  while ($row = db_fetch_array($result)) {
    $accum($ret, $row);
  }
  return $ret;
}

function best_matches($str, $strings) {
  $dists = array_map(function($target) use($str) { return levenshtein($str, $target); },
                     $strings);
  asort($dists);
  print_r($dists);
  array_walk(&$dists, function(&$d, $id) use($strings) {$d = $strings[$id];});
  return $dists;
}

$node_urls = proc_rows('SELECT nid FROM {node}',
                       function(&$ar, $row) { $ar[] = drupal_lookup_path('alias', 'node/' . reset($row)); });

$node_urls = array_filter($node_urls);

$bad_links = proc_rows('SELECT lid,url FROM {linkchecker_links} where code=404',
                       function(&$ar, $row) { $ar[$row['lid']] = $row['url']; });

print_r($node_urls);
print "\n";

print_r ($bad_links);
print "\n";

$replaces=array();
foreach ($bad_links as $lid => $link) {
  $res = best_matches($link, $node_urls);
  print_r($res);
  $inp = drush_choice(reset(array_chunk($res, 9, true)), $link);
  print "selected " . $inp . " " . $res[$inp] . "\n";
}

foreach($replaces as $url_id => $info) {
  print $node_urls[$url_id] . " =(". $info['dist']  ."=> " . $bad_links[$info['lid']] . "\n" ;
}

print count($replaces) . "\n";
print count(array_unique($replaces)) . "\n";