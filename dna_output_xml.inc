<?php
require_once 'dna_output_interface.inc';

/**
 * Decorator class around dna_settings
 */
class dna_xml extends dna_settings {


  function __construct() {
    parent::__construct();
  }

  /**
   * @override
   *
   * Outputs the module info as xml
   */
  public function get_output() {
    $ret = array();
    $ret['timestamp'] = $this->timestamp;
    $ret['variables'] = $this->variables;
    $ret['modules'] = $this->modules;
    $xml = $this->toXML($ret);
    return $this->format_xml($xml);
  }


  /**
   * Parses php array into xml
   *
   * mostly copied from http://snipplr.com/view.php?codeview&id=3491
   *
   * @todo damn views object
   */
  private function toXML(array $data, $rootNodeName = 'data', $xml=null) {
    // turn off compatibility mode as simple xml throws a wobbly if you don't.
    if (ini_get('zend.ze1_compatibility_mode') == 1) {
      ini_set ('zend.ze1_compatibility_mode', 0);
    }

    if ($xml == null) {
      $xml = simplexml_load_string("<?xml version='1.0' encoding='utf-8'?><$rootNodeName />");
    }

    // loop through the data passed in.
    foreach($data as $key => $value) {

       if (empty($value)) {
         continue;
       }

      // no numeric keys in our xml please!
      if (is_numeric($key)) {
        $key = 'element';
      }

      // replace anything not alpha numeric or '_'
      $key = preg_replace('/[^a-z_]/i', '', $key);

      if (is_array($value)) {
        $node = $xml->addChild($key);
        // recrusive call.
        $this->toXml($value, $rootNodeName, $node);
      } else {
        // add single node.
        $value = htmlentities($value);
        $xml->addChild($key, $value);
      }

    }

    return $xml;
  }


  /**
   * Formats xml to be well structured, indented
   * and higlighted as html
   *
   * @return String html output
   */
  private static function format_xml(SimpleXMLElement $xml) {
    // Only domdocument can indent xml, convert
    $dom = dom_import_simplexml($xml)->ownerDocument;
    $dom->formatOutput = true;
    $xml_string = $dom->saveXML(); 

    require_once drupal_get_path('module', 'geshifilter') .'/geshifilter.pages.inc';
    return geshifilter_process($xml_string, 'XML');
  }

}
