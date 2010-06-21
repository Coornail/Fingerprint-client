<?php
require_once 'dna_output_interface.inc';

class php_export implements dna_output {
  /**
   * @var Internal storage of the settings
   */
  private $settings;

  function __construct($settings) {
    $this->settings = $settings;

  }

  public function get_output() {
    return array();
  }

}
