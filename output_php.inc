<?php
require_once 'output_interface.inc';

class php_export implements dna_output {
  /**
   * @var Internal storage of the settings
   */
  private $settings;

  function __construct($settings) {

  }

  public function get_output() {
    return array();
  }

}
