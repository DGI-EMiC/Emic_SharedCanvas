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
  module_load_include('inc', 'fedora_repository', 'ContentModel');

  $cm_object = ContentModel::loadFromObject($pid);
  $cModel = $cm_object->pid;

  if ($cModel == 'islandora:criticalEditionCModelPage') {
    $item = new Fedora_Item($pid);
    $rels = $item->get_rdf_relationships(ISLANDORA_RELS_EXT_URI);
    $pid = $rels['isEditionOf'][0];
  }

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
  $results['islandora_get_annotation'] = "$base_url/emic/get_annotation/";
  $results['islandora_delete_annotation'] = "$base_url/emic/delete_annotation/$pid/";
  $results['get_annotation_list_url'] = "$base_url/emic/get_urns/$pid";
  $results['islandora_update_annotation'] = "$base_url/emic/update_annotation/";
  $results['position'] = $position;
  $results['pages'] = $pages;
  $results['no_edit'] = !user_access('edit CWRC');
  $json = json_encode($results);
  echo $json;
}

/**
 * Creates annotation objects from html supplied by Shared Canvas
 * @global url $base_url
 * @param string $pid
 */
function islandora_add_annotation($pid) {
  global $user;
  global $base_url;
  module_load_include('inc', 'fedora_repository', 'api/fedora_item');
  module_load_include('inc', 'emic_canvas', 'islandora_rdf_builder');
  module_load_include('inc', 'emic_canvas', 'build_sharedcanvas_streams');

  $annotation_xml = emic_shared_transform_annotationList(urldecode($_POST['data']), 'html');
  $type = $_POST['type'];
  if (!$type) {
    $type = 'unclassified';
  }
  $xml = simplexml_load_string($annotation_xml);
  $labels = $xml->xpath('//dc:title');
  $label = (string) $labels[0];

  $namespace = substr($pid, 0, strpos($pid, ":"));
  $annotation_pid = FEDORA_ITEM::get_next_PID_in_namespace($namespace);
  $item = fedora_item::ingest_new_item($annotation_pid, 'A', $label, $user->name);
  $item->add_relationship('hasModel', 'islandora:OACCModel', FEDORA_MODEL_URI);
  $item->add_relationship('targetedBy', $user->name, ISLANDORA_RELS_EXT_URI);
  $item->add_relationship('targets', $pid, ISLANDORA_RELS_EXT_URI);
  $item->add_relationship('isAnnotationType', $type, 'http://islandora.ca/ontology/relsext#', 1);
  $item->add_datastream_from_string($annotation_xml, 'OAC', 'OAC', 'text/plain', 'X');
  $annotations = $xml->xpath('//rdf:type[@rdf:resource="http://www.openannotation.org/ns/Annotation"]');
  $parents = $annotations[0]->xpath("parent::*");
  $parent = $parents[0];
  sleep(1);
  foreach ($parent->attributes('rdf', TRUE) as $key => $value) {
    echo (string) $value;
  }
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
 * Gets individual annotation urns from solr
 * called with emic/get_urns
 * @global url $base_url
 * @param <type> $pid
 */
function islandora_shared_get_annotation_urns($pid) {

  $pid = str_replace(':', '\:', $pid);
  $solr = variable_get('islandora_solr_search_block_url', 'http://localhost:8080/solr');

  $url = "http://$solr/select?indent=on&version=2.2&q=constrains_canvas%3A$pid&fq=&start=0&rows=10&fl=annotation_urn&qt=standard&wt=json&explainOther=&hl.fl=";
  $results = json_decode(file_get_contents($url), TRUE);
  $candidates = $results['response']['docs'];
  foreach ($candidates as $candidate) {
    $urns[] = $candidate['annotation_urn'][0];
  }
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
function get_annotation_data($urn) {
  module_load_include('inc', 'fedora_repository', 'api/fedora_item');
  global $base_url;
  $annotation_pid = emic_canvas_get_pid_from_urn($urn);
  $item = new Fedora_Item($annotation_pid);
  if (!$item->exists()) {
    return;
  }
  $annotation_xml = $item->get_datastream_dissemination('OAC');
  return $annotation_xml;
}

/**
 * Retrieves transforms annotation data for the editor
 * Called with /emic/get_annotation/
 * Doctype declarations are removed
 * @param string $pid
 * @param urn $urn
 */
function islandora_shared_get_annotation($urn) {

  $xml = get_annotation_data($urn);
  $results = emic_shared_transform_annotationList($xml, 'rdf');
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
  $pid_to_delete = emic_canvas_get_pid_from_urn($urn);
  $item = new Fedora_Item($pid_to_delete);
  $item->purge();
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

/**
 * Gets the pid of an annotation object identified by a know urn
 * @param URN $urn
 * @return PID
 */
function emic_canvas_get_pid_from_urn($urn) {
  $solr = variable_get('islandora_solr_search_block_url', 'http://localhost:8080/solr');
  $urn = str_replace(':', '\:', $urn);
  $url = "http://$solr/select?indent=on&version=2.2&q=annotation_urn%3A$urn&fq=&start=0&rows=10&fl=PID&qt=standard&wt=json&explainOther=&hl.fl=";
  $results = json_decode(file_get_contents($url), TRUE);
  $annotation_pid = $results['response']['docs'][0]['PID'];
  return $annotation_pid;
}

/**
 * Updates existing annotation
 * Called by Shared Canvas with ajax
 */
function emic_canvas_update_annotation() {
  module_load_include('inc', 'fedora_repository', 'api/fedora_item');
  $changed = FALSE;
  //retrieve variables from POST
  $urn = urldecode($_POST['urn']);
  $title = urldecode($_POST['title']);
  $content = urldecode($_POST['content']);
  $type = urldecode($_POST['annoType']);
  $pid = emic_canvas_get_pid_from_urn($urn);

  $item = new Fedora_Item($pid);
  $annotation = $item->get_datastream_dissemination('OAC');
  $xml = simplexml_load_string($annotation);

  // identify Title element and replace content
  $title_node = $xml->xpath("//rdf:Description[@rdf:about = 'urn:uuid:$urn']/dc:title");
  if ($title_node[0][0] != $title) {
    $title_node[0][0] = $title;
    $changed = TRUE;
  }

    // identify Type element and replace content
  $type_node = $xml->xpath("//rdf:Description[@rdf:about = 'urn:uuid:$urn']/dc:type");
  if ($type_node[0][0] != $type) {
    $type_node[0][0] = $type;
    $changed = TRUE;
  }
  //indentify URN of body to retrieve chars node to hold new annotation content
  $body_urn_array = $xml->xpath("//oac:hasBody");
  $body_urn = (string) $body_urn_array[0]->attributes('rdf', 'about');
  $content_node = $xml->xpath("//rdf:Description[@rdf:about = '$body_urn']//cnt:chars");
  if ((string) $content_node[0][0] != $content) {
    $content_node[0][0] = $content;
    $changed = TRUE;
  }


  // persist updated annotation
  if ($changed) {
    $item->modify_datastream($xml->asXML(), 'OAC', 'OAC', 'text/plain', FALSE);
  }
}

