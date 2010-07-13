<?php

/**
 * Callback for node/%/fingerprint_reverse_diff
 */
function fingerprint_similar_list($node) {
  global $user;
  $other_nodes = array();

  if (!is_object($node)) {
    $node = node_load($node);
  }
  $result = db_query(db_rewrite_sql('SELECT nid, title FROM {node} WHERE type="fingerprint" AND nid<>%d'), $node->nid);
  while ($other_node= db_fetch_object($result)) {
    $other_nodes[] = l($other_node->title, 'fingerprint_similar_view/'. $node->nid .'/'. $other_node->nid);
  }

  return theme('item_list', $other_nodes);
}

