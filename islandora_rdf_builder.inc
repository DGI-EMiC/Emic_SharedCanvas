<?php

/**
 * Convenicence class to build RDF objects
 *
 */
class IslandoraRdfBuilder {

  public $domDoc = null;

  function IslandoraRdfBuilder($domDoc = null) {
    if (!isset($domDoc)) {
      $this->domDoc = new DOMDocument('1.0', 'utf-8');
      $this->domDoc->formatOutput = true;
      $this->root = $this->domDoc->appendChild($this->domDoc->createElement('rdf:RDF'));
      $this->root->setAttribute('xmlns:dc', "http://purl.org/dc/elements/1.1/");
      $this->root->setAttribute('xmlns:dcterms', "http://purl.org/dc/terms/");
      $this->root->setAttribute('xmlns:dms', "http://dms.stanford.edu/ns/");
      $this->root->setAttribute('xmlns:foaf', "http://xmlns.com/foaf/0.1/");
      $this->root->setAttribute('xmlns:ore', "http://www.openarchives.org/ore/terms/");
      $this->root->setAttribute('xmlns:rdf', "http://www.w3.org/1999/02/22-rdf-syntax-ns#");
    }
    else {
      $this->domDoc = $domDoc;
      $this->root = $this->domDoc->documentElement;
    }
  }

  /**
   * Returns value of IslandoraRdfBuilder object as an xml string
   * @return string
   */
  function printRDF() {
    return $this->domDoc->saveXML();
  }

  /**
   * Add description node to existing RDF XML
   * @param string $about
   * the resource identified in the rdf 'about' attribute
   * @param array $children
   * A structured array of child elements to this node
   * Each array entry is a sub array with a manditory name, an optional array of attributes, and an optional value
   */
  function addDescriptionNode($about, $children = null) {
    $description = $this->domDoc->createElement('rdf:Description');
    $description = $this->root->appendChild($description);
    $description->setAttribute("rdf:about", $about);

    foreach ($children as $child) {
      $new = $this->domDoc->createElement($child['name']);
      $description->appendChild($new);
      if (isset($child['value'])) {
        $value = $this->domDoc->createTextNode($child['value']);
        $new->appendChild($value);
      }
      if ($child['attributes']) {
        foreach ($child['attributes'] as $key => $attVal) {
          $new->setAttribute($key, $attVal);
        }
      }
    }
  }


  /**
   * Takes a Nodelist of Description extracted from external RDF document and adds
   * them to this object's DOMDocument
   * @param NodeList $nodeList
   */
  function addDescriptionsFromNodelist($nodeList) {
    foreach ($nodeList as $domElement) {
      $domNode = $this->domDoc->importNode($domElement, true);
      $this->root->appendChild($domNode);
    }
  }


    /**
   * Takes a Description element extracted from external RDF document and adds
   * it to this object's DOMDocument
   * @param NodeList $nodeList
   */
  function addDescriptionElement($domElement) {
      $domNode = $this->domDoc->importNode($domElement, true);
      $this->root->appendChild($domNode);
  }

  function deleteDescriptionElement($node_id){
    $xpath = new DOMXPath($this->domDoc);
    $candidates = $xpath->query("//rdf:Description[@rdf:about='$node_id']");
    foreach($candidates as $candidate){
      //$parent = $candidate->parentNode;
     // $parent->removeChild($candidate);
      $candidate->parentNode->removeChild($candidate);    
    }
  }
  
  /**
   * Convenience method to structure input arrays for addDescriptionNode
   * @param string $name
   * Name of child tag
   * @param array $attributes
   * Array of key => value pairs (optional)
   * @param string $value
   * vale for tag (optional)
   * @return array
   */
  function build_child($name, $attributes= null, $value = null) {
    $return_array = array();
    $return_array['name'] = $name;
    if ($value) {
      $return_array['value'] = $value;
    }
    if ($attributes) {
      foreach ($attributes as $key => $attVal) {
        $return_array['attributes'][$key] = $attVal;
      }
    }
    return $return_array;
  }

}

