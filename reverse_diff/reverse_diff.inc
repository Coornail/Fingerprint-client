<?php


class ReverseDiffFormatter extends DrupalDiffFormatter {

  /**
   * Override maximum context line values (so it shows allt he nonchanged content)
   */
  function __construct() {
    $this->leading_context_lines = PHP_INT_MAX;
    $this->trailing_context_lines = PHP_INT_MAX;
  }

  function addedLine($line) {
    return array(
      ' ',
      array(
        'data' => '<br />',
      )
    );
  }

  function deletedLine($line) {
    return $this->addedLine($line);
  }

}
