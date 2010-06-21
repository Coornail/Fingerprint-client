<?php

/**
 * Interface for output plugins for the dna export
 */
interface dna_output {
  function __construct();

  /**
   * Output provider
   *
   * @returns String The desired output
   */
  public function get_output();
}
