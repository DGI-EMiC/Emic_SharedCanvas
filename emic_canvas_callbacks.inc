<?php

/**
 * Ajax callback for Shared Canvas
 * Supplies the basic data, including maifest uri
 * @global url $base_url
 * @param string $pid
 */
function islandora_canvas_setup($pid) {
  global $base_url;

  global $user;
  module_load_include('inc', 'islandora_book', 'book_pack_utils');
  module_load_include('inc', 'fedora_repository', 'api/fedora_item');
  module_load_include('inc', 'emic_canvas', 'build_sharedcanvas_streams');
  module_load_include('inc', 'emic_canvas', 'islandora_rdf_builder');
  $books = get_collection_from_pid($pid);
  $book = $books[0];
  $item = new Fedora_Item($book);
  $title = $item->objectProfile->objLabel;
  $pages = get_sorted_pages($book);
  $position = array_search($pid, $pages);
  $item = new Fedora_Item($pid);
  $results = array();
  $results['manifest_url'] = "$base_url/emic/serve/$pid/Manifest/manifest.xml";
  $results['page_title'] = $item->objectProfile->objLabel;
  $results['object_base'] = "$base_url/fedora/repository/$pid";
  $results['islandora_post_url'] = "$base_url/emic/shared/add_annotation/$pid";
  $results['islandora_get_annotation'] = "$base_url/emic/get_annotation/$pid/";
  $results['islandora_delete_annotation'] = "$base_url/emic/delete_annotation/$pid/";
  $results['get_annotation_list_url'] = "$base_url/emic/get_urns/$pid";
  $results['position'] = $position;
  $results['pages'] = $pages;
  $results['no_edit'] = !user_access('edit CWRC');
  $json = json_encode($results);
  echo $json;
}

/**
 * Adds annotation created in editor to saved datastream
 * @global url $base_url
 * @param string $pid
 */
function islandora_add_annotation($pid) {
  global $base_url;
  $doc_url = "$base_url/fedora/repository/$pid";
  $doc_path_url = "$base_url/emic/serve/$pid";
  module_load_include('inc', 'fedora_repository', 'api/fedora_item');
  module_load_include('inc', 'emic_canvas', 'islandora_rdf_builder');
  module_load_include('inc', 'emic_canvas', 'build_sharedcanvas_streams');
// create domdoc and use it to populate builder
  $item = new Fedora_Item($pid);
  $domDoc = new DOMDocument();
  if (!key_exists('AnnotationList', $item->datastreams)) {
    $list = get_annotationList($doc_url, $doc_path_url);
  }
  else {
    $list = $item->get_datastream_dissemination('AnnotationList');
  }
  $domDoc->loadXML($list);
  $rdf = new IslandoraRdfBuilder($domDoc);
  $out = $rdf->printRDF();
// create Domdoc object with transformed data from SharedCanvas
  $annotationDomDoc = new DOMDocument();
  $annotation_xml = emic_shared_transform_annotationList(urldecode($_POST['data']), 'html');
  $simplexml = simplexml_load_string($annotation_xml);
  $annotations = $simplexml->xpath('//rdf:type[@rdf:resource="http://www.openannotation.org/ns/Annotation"]');
  $parents = $annotations[0]->xpath("parent::*");
  $parent = $parents[0];
  foreach ($parent->attributes('rdf', TRUE) as $key => $value) {
    echo (string) $value;
  }

  $annotationDomDoc->loadXML($annotation_xml);

// Get Description elements as nodeList and send them to
  $descriptions = $annotationDomDoc->getElementsByTagName('Description');
  $rdf->addDescriptionsFromNodelist($descriptions);
  $out = $rdf->printRDF();

  islandora_save_updated_annotations($pid, $rdf);
}

/**
 * transforms input from SharedCanvas to AnnotationList
 * @param string $xml_in
 * @return string
 */
function emic_shared_transform_annotationList($xml_in, $input_type) {

  if ($xml_in == NULL || strlen($xml_in) < 5) {
    return " ";
  }

  $path = drupal_get_path('module', 'emic_canvas') . '/xslt/';
  $xslts = array(
    'html' => $path . 'html_to_AnnotationList.xsl',
    'rdf' => $path . 'AnnotationList_to_html.xsl'
  );
  try {
    $proc = new XsltProcessor();
  } catch (Exception $e) {
    drupal_set_message(t('@e', array('@e' => check_plain($e->getMessage()))), 'error');
    return " ";
  }

  $xsl = new DomDocument();
  $xsl->load($xslts[$input_type]);
  $input = new DomDocument();
  $input->loadXML(trim($xml_in));
  $xsl = $proc->importStylesheet($xsl);
  $newdom = $proc->transformToDoc($input);
  $return_xml = $newdom->saveXML();

  return $return_xml;
}

/**
 * Gets individual annotatio  urns from datastream
 * @global url $base_url
 * @param <type> $pid
 */
function islandora_shared_get_annotation_urns($pid) {
  $rdf = build_annotationList($pid);

  $descriptions = $rdf->domDoc->getElementsByTagName('Description');

  foreach ($descriptions as $description) {
    $about = $description->getAttribute('rdf:about');
    $types = $description->getElementsByTagName('type');
    foreach ($types as $type) {
      if ($type->getAttribute('rdf:resource') == 'http://www.openannotation.org/ns/Annotation') {
        $urns[] = $about;
      }
    }
  }
  $results = array();
  $results['info'] = $urns;
  $json = json_encode($urns);
  echo $json;
}

/**
 * Retrieves details of individual annotation from datastream
 * @global url $base_url
 * @param string $pid
 * @param urn $urn
 * @return IslandoraRdfBuilder
 */
function get_annotation_data($pid, $urn) {

  global $base_url;
  module_load_include('inc', 'fedora_repository', 'api/fedora_item');
  module_load_include('inc', 'emic_canvas', 'islandora_rdf_builder');
  $item = new Fedora_Item($pid);
  $domDoc = new DOMDocument();
  $list = $item->get_datastream_dissemination('AnnotationList');
  $domDoc->loadXML($list);
  $rdf = new IslandoraRdfBuilder($domDoc);

  $descriptions = $rdf->domDoc->getElementsByTagName('Description');
  $count = $descriptions->length;
  $body_urns = array();
  $target_urns = array();
  $constraint_urns = array();
  foreach ($descriptions as $description) {
    if ($urn == $description->getAttribute('rdf:about')) {    // gets description element matching URN
      $rdf->addDescriptionElement($description);              // add this element to new RDF object
      $bodies = $description->getElementsByTagName('hasBody'); // get body element from Annotation object
      foreach ($bodies as $body) {                            // get urns from hasBody elements
        $body_urns[] = $body->getAttribute("rdf:resource");
      }
      $targets = $description->getElementsByTagName('hasTarget');
      foreach ($targets as $target) {                           // gets urns from target elements
        $target_urns[] = $target->getAttribute("rdf:resource");


        foreach ($descriptions as $description) {
          $about = $description->getAttribute('rdf:about');
          if (in_array($description->getAttribute('rdf:about'), $target_urns)) {
            $contraints = $description->getElementsByTagName('constrainedBy');
            foreach ($contraints as $contraint) {                            // get urns from hasBody elements
              $constraint_urns[] = $contraint->getAttribute("rdf:resource");
            }
          }
        }
      }
      foreach ($descriptions as $description) {
        $about = $description->getAttribute('rdf:about');
        if (in_array($description->getAttribute('rdf:about'), array_merge($body_urns, $target_urns, $constraint_urns))) {
          $rdf->addDescriptionElement($description);
        }
      }
    }
  }
  return $rdf;
}

/**
 * Retrieves transforms annotation data for the editor
 * Doctype declarations are removed
 * @param string $pid
 * @param urn $urn
 */
function islandora_shared_get_annotation($pid, $urn) {

  $rdf = get_annotation_data($pid, $urn);
  $results = emic_shared_transform_annotationList($rdf->printRDF(), 'rdf');
  $results = preg_replace('/<\?xml.*?\?>/', '', $results);
  $results = preg_replace('/<!DOCTYPE.*?>/', '', $results);

  echo $results;
}

/**
 *

 * @param string $pid
 * @param urn $urn
 */
function islandora_delete_annotation($pid, $urn) {

  module_load_include('inc', 'fedora_repository', 'api/fedora_item');
  module_load_include('inc', 'emic_canvas', 'islandora_rdf_builder');
// create domdoc and use it to populate builder
  $item = new Fedora_Item($pid);
  $domDoc = new DOMDocument();
  $list = $item->get_datastream_dissemination('AnnotationList');
  $domDoc->loadXML($list);
  $rdf = new IslandoraRdfBuilder($domDoc);
  $body_urn = $rdf->getAnnotationBodyUrn("urn:uuid:$urn");
  $target_urn = $rdf->getAnnotationTargetUrn("urn:uuid:$urn");
  $constrained_target_urn = $rdf->getConstrainedTarget($target_urn);

  $rdf->deleteDescriptionElement("urn:uuid:$urn"); //delete the orginal annotation
  $rdf->deleteDescriptionElement($body_urn);
  $rdf->deleteDescriptionElement($target_urn);
  $rdf->deleteDescriptionElement($constrained_target_urn);
  islandora_save_updated_annotations($pid, $rdf);
}

/**
 * Persists new annotations
 * @global url $base_url
 * @param <type> $pid
 * @param <type> $rdf
 */
function islandora_save_updated_annotations($pid, $rdf) {
  global $base_url;
  module_load_include('inc', 'fedora_repository', 'api/fedora_item');
  $item = new Fedora_Item($pid);
  $updated_annotationList = $rdf->printRDF();
  $flat_pid = str_replace(':', '_', $pid);
  $file_path = file_directory_path() . "/$flat_pid";
  $file = "$file_path/annotationList.xml";
  if (!is_dir($file_path)) {
    mkdir($file_path, 0777, TRUE);
  }

  file_put_contents($file, $updated_annotationList);
  $item->add_or_modify_by_reference("$base_url/$file", 'AnnotationList', "AnnotationList", 'text/plain', FALSE, 'Modified by Islandora API');

  file_delete("$file");
  rmdir($file_path);
}

function build_annotationList($pid) {
  global $base_url;
  $doc_url = "$base_url/fedora/repository/$pid";
  $doc_path_url = "$base_url/emic/serve/$pid";
  module_load_include('inc', 'fedora_repository', 'api/fedora_item');
  module_load_include('inc', 'emic_canvas', 'build_sharedcanvas_streams');
  module_load_include('inc', 'emic_canvas', 'islandora_rdf_builder');
  $item = new Fedora_Item($pid);

  $annotationList = get_annotationList($doc_url, $doc_path_url);
  $domDoc = new DOMDocument();
  $domDoc->formatOutput = true;
  $domDoc->loadXML($annotationList);
  $rdf = new IslandoraRdfBuilder($domDoc);

  if (array_key_exists('AnnotationList', $item->datastreams)) {
    $list = $item->get_datastream_dissemination('AnnotationList');
    if (strlen($list) < 50) {
      return;
    }
    $annoDoc = new DOMDocument();
    $annoDoc->loadXML($list);
    $descriptions = $annoDoc->getElementsByTagName('Description');
    foreach ($descriptions as $description) {
      $constrains = $description->getElementsByTagName('constrains');
      foreach ($constrains as $candidate) {
        $candidate->setAttribute("rdf:resource", "$base_url/fedora/repository/$pid/Canvas");
      }
      $newNode = $domDoc->importNode($description, TRUE);
      $rdf->root->appendChild($newNode);
    }
  }
  return $rdf;
}