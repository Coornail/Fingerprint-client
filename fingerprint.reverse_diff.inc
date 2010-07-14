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
 *
 * @todo Merge this and parts of fingerprint_view_diff()
 */
function fingerprint_view_similar() {
  $nodes = arg();
  array_shift($nodes);

  $current = node_load($nodes[0]);
  $other = node_load($nodes[1]);

  drupal_set_title(t('Similar rows in @origin and @other', array('@origin' => $current->title, '@other' => $other->title)));

  // Emulate node render
  fingerprint_nodeapi(&$current, 'view', NULL, NULL, FINGERPRINT_NOHIGHLIGHT);
  fingerprint_nodeapi(&$other, 'view', NULL, NULL, FINGERPRINT_NOHIGHLIGHT);
  // Decoding '<' and '>' chars so it can be decoded
  $current->body = htmlspecialchars_decode($current->content['body']['#value']);
  $other->body = htmlspecialchars_decode($other->content['body']['#value']);

  $other->body = explode("\n", $other->body);
  $current->body = explode("\n", $current->body);

  $diff = new Diff($other->body, $current->body);
  $diff->Diff($other->body, $current->body);

  $formatter = new ReverseDiffFormatter();
  $diff_rows = $formatter->format($diff);

  return theme('fingerprint_reverse_diff', $diff_rows);
}


/**
 * Displays the same lines in two nodes
 *
 * @param $rows
 *   Array of rows to display
 *
 * @todo fix trailing spaces
 */
function theme_fingerprint_reverse_diff($rows) {
  $xml = implode("\n", $rows);
  module_load_include('inc', 'fingerprint', 'fingerprint_output_xml');
  return FingerprintXml::formatXml($xml);
}


module_load_include('php', 'diff', 'DiffEngine'); // needed for DrupalDiffFormatter class
/**
 * Show the matching lines in a diff
 * @subpackage DifferenceEngine
 */
class ReverseDiffFormatter extends DrupalDiffFormatter {

  /**
   * Override maximum context line values (so it shows all the nonchanged content)
   */
  function __construct() {
    $this->leading_context_lines = PHP_INT_MAX;
    $this->trailing_context_lines = PHP_INT_MAX;
  }


  /**
   * @override
   *
   * Handles hwo the objects will be written to the screen
   * @todo better separator
   */
  function _block($xbeg, $xlen, $ybeg, $ylen, &$edits) {
    foreach ($edits as $edit) {
      if ($edit->type == 'copy') {
        $this->rows = array_merge ($this->rows, $edit->orig);
      } else {
        $this->rows[] = ''; // empty line for separating
      }
    }
  }

}

