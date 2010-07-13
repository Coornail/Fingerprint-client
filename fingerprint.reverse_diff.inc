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



/**
 * Callback for fingerprint_similar_view
 */
function fingerprint_similar_view() {
  $nodes = arg();
  array_shift($nodes);

  $current = node_load($nodes[0]);
  $other = node_load($nodes[1]);

  // Emulate node render
  fingerprint_nodeapi(&$current, 'view', NULL, NULL, FINGERPRINT_NOHIGHLIGHT);
  fingerprint_nodeapi(&$other, 'view', NULL, NULL, FINGERPRINT_NOHIGHLIGHT);
  // Decoding '<' and '>' chars so it can be decoded
  $current->body = htmlspecialchars_decode($current->content['body']['#value']);
  $other->body = htmlspecialchars_decode($other->content['body']['#value']);

  module_load_include('php', 'diff', 'DiffEngine');
  module_load_include('inc', 'fingerprint', 'reverse_diff/reverse_diff');

  //$diff = new ReverseDiff($other->body, $current->body);
  $other->body = explode("\n", $other->body);
  $current->body = explode("\n", $current->body);

  $diff = new ReverseDiff($other->body, $current->body);
  $diff->Diff($other->body, $current->body);

  $formatter = new DrupalDiffFormatter();
  $diff_rows = $formatter->format($diff);

  module_load_include('inc', 'diff', 'diff.pages');
  $header = _diff_default_header();
  $cols = _diff_default_cols();

  drupal_add_css(drupal_get_path('module', 'diff') .'/diff.css', 'module', 'all', FALSE);
  return theme('diff_table', $header, $diff_rows, array('class' => 'diff'), NULL, $cols);
}

