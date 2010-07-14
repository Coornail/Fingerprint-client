<?php

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
