<?php

// Separates the similar blocks from eachother
define('FINGERPRINT_REVERSE_DIFF_SEPARATOR', "\n--------------------------------------------------------------------------------\n");
// Number of maximum nodes to show in listings
define('FINGERPRINT_REVERSE_DIFF_MAX_NODES', 10);

/**
 * Callback for node/%/fingerprint_reverse_diff and block[0]
 * Shows a list of nodes to find similarities
 *
 * @param $nodes
 *   A node id, object, or an array of node ids
 *
 * @todo
 *   - Write my own pager (just prev and next)
 */
function fingerprint_similar_list($nodes) {
  global $user;
  $other_nodes = array();

  if (is_int($nodes) || is_string($nodes)) {
    $nodes = array((int)$nodes);
  }

  if (is_object($nodes)) {
    $nodes = array($nodes->nid);
  }

  $path = 'fingerprint_similar_view/' . implode('/', $nodes) . '/';

  $result = pager_query(db_rewrite_sql('SELECT nid, title FROM {node} WHERE type="fingerprint" AND nid NOT IN('. db_placeholders($nodes, 'int') .')'), FINGERPRINT_REVERSE_DIFF_MAX_NODES, 0, NULL, $nodes);
  while ($other_node = db_fetch_object($result)) {
    $other_nodes[] = l($other_node->title, $path . $other_node->nid);
  }

  return theme('item_list', $other_nodes) . theme('pager');
}



/**
 * Callback for fingerprint_similar_view
 *
 * @todo Merge this and parts of fingerprint_view_diff()
 */
function fingerprint_view_similar() {
  $nids = arg();
  array_shift($nids);

  $nodes = array();
  foreach ($nids as $nid) {
    $nodes[] = node_load($nid); 

  }

  // If there are more than 2 nodes to compare, don't put it in the title
  $node_titles = '';
  if (count($nodes) === 2) {
    drupal_set_title(t('Similar patterns in @origin and @other', array('@origin' => $nodes[0]->title, '@other' => $nodes[1]->title)));
  } else {
    drupal_set_title(t('Similar patterns in'));
    foreach ($nodes as $node) {
      $node_titles[] = l($node->title, 'node/'. $node->nid);
    }
  }

  // Emulate node render
  foreach ($nodes as &$node) {
    fingerprint_nodeapi(&$node, 'view', NULL, NULL, FINGERPRINT_NOHIGHLIGHT);
    // Decoding '<' and '>' chars so it can be displayed
    $node->body = htmlspecialchars_decode($node->content['body']['#value']);
    $node->body = explode("\n", $node->body);
  }


  $node_number = count($nodes)-1;
  for ($i=0; $i<$node_number; $i++) {
    $diff = new Diff($nodes[$i]->body, $nodes[$i+1]->body);
    $diff->Diff($nodes[$i]->body, $nodes[$i+1]->body);
    $formatter = new ReverseDiffFormatter();
    $nodes[$i+1]->body = $formatter->format($diff);
  }

  $blocks = $formatter->blocks;
  foreach ($blocks as &$block) {
    $block = implode("\n", $block);
  }

  $similar = implode(FINGERPRINT_REVERSE_DIFF_SEPARATOR, $blocks);
  return (count($node_titles) ? theme('item_list', $node_titles) : '') . theme('fingerprint_reverse_diff', $similar);
}


/**
 * Displays the same lines in two nodes
 *
 * @param $rows
 *   Array of rows to display
 */
function theme_fingerprint_reverse_diff($rows) {
  //$xml = implode("\n", $rows);
  module_load_include('inc', 'fingerprint', 'fingerprint_output_xml');
  return FingerprintXml::formatXml($rows);
}


module_load_include('php', 'diff', 'DiffEngine'); // needed for DrupalDiffFormatter class
/**
 * Show the matching lines in a diff
 * @subpackage DifferenceEngine
 */
class ReverseDiffFormatter extends DrupalDiffFormatter {

  public $blocks = array();

  /**
   * Override maximum context line values (so it shows all the nonchanged content)
   */
  public function __construct() {
    $this->leading_context_lines = PHP_INT_MAX;
    $this->trailing_context_lines = PHP_INT_MAX;
  }


  /**
   * @override
   *
   * Handles hwo the objects will be written to the screen
   */
  public function _block($xbeg, $xlen, $ybeg, $ylen, &$edits) {
    foreach ($edits as $edit) {
      if ($edit->type == 'copy') {
        $this->rows = array_merge ($this->rows, $edit->orig);
        $this->blocks[] = $edit->orig;
      } else {
       // $this->rows[] = FINGERPRINT_REVERSE_DIFF_SEPARATOR; // empty line for separating
      }
    }
  }

}

