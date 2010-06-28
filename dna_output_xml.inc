<?php
module_load_include('inc', 'dna', 'dna_settings');

/**
 * @package dna
 *
 * Outputs the site settings as xml
 */
class dna_xml extends dna_settings {


  function __construct($settings = array()) {
    parent::__construct($settings);
  }


  /**
   * @override
   *
   * Outputs the module info as xml
   */
  public function get_output() {
    $ret = $this->get_output_items();
    $xml = $this->toXML($ret);

    // Only domdocument can indent xml, convert
    $dom = dom_import_simplexml($xml)->ownerDocument;
    $dom->formatOutput = TRUE;
    return $dom->saveXML(); 
  }


  /**
   * Parses php array into xml
   *
   * mostly copied from http://snipplr.com/view.php?codeview&id=3491
   */
  private function toXML(array $data, $rootNodeName = 'data', $xml = null) {
    // turn off compatibility mode as simple xml throws a wobbly if you don't.
    if (ini_get('zend.ze1_compatibility_mode') == 1) {
      ini_set('zend.ze1_compatibility_mode', 0);
    }

    if ($xml == null) {
      $xml = simplexml_load_string('<?xml version="1.0" encoding="utf-8"?><'. $rootNodeName .' /> ');
    }

    foreach ($data as $key => $value) {
       if (empty($value)) {
         continue;
       }

      // no numeric keys
      if (is_numeric($key)) {
        $key = 'element';
      }

      // __PHP_Incomplete_Class is not is_object(), have to workaround... 
      if (is_object($value) || !(is_int($value) || is_string($value) || is_array($value) || is_bool($value)) ) {
        dna_settings::object_to_array_recursive($value);
      }

      // replace anything not alpha numeric or '_'
      $key = preg_replace('/[^a-z_]/i', '', $key);

      if (is_array($value)) {
        $node = $xml->addChild($key);
        $this->toXml($value, $rootNodeName, $node);
      }
      else {
        // add single node.
        $value = htmlentities($value);
        $xml->addChild($key, $value);
      }

    }

    return $xml;
  }


  /**
   * Formats and highlights xml
   * 
   * Tries highlight if geshifilter is enabled
   * Cleans up xml if tidy extension is installed
   *
   * @return String html output
   */
  public static function format_xml($xml) {
    // Check for broken xml (for example node teaser)
    if (extension_loaded('tidy')) {
      $config = array(
        'indent'         => true,
        'output-xml'     => true,
        'input-xml'     => true,
        'wrap'         => '1000'
      );

      $tidy = new tidy();
      $tidy->parseString($xml, $config, 'utf8');
      $tidy->cleanRepair();
      $xml = tidy_get_output($tidy);
    }

    if (module_exists('geshifilter')) {
      require_once drupal_get_path('module', 'geshifilter') .'/geshifilter.pages.inc';
      return geshifilter_process($xml, 'XML');
    }

    return str_replace("\n", '<br />', htmlspecialchars($xml));
  }

}
