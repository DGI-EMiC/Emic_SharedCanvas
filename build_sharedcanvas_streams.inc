<?php

/**
 * Builds initial suite of xml's for Shared Canvas installation
 *
 * @global url $base_url
 * @param string $pid
 *
 */
function build_manifest($pid) {
  global $base_url;
  module_load_include('inc', 'fedora_repository', 'api/fedora_item');
  $item = new Fedora_Item($pid);
  $title = $item->objectProfile->objLabel;
  $file_array = array();
  $doc_url = "$base_url/fedora/repository/$pid";
  //xml datastreams used by Shared Canvas must be echoed though ajax.
  $doc_path_url = "$base_url/emic/serve/$pid";
  $dimensions = get_height_and_width($doc_url . '/JPEG/');

  $height = $dimensions['height'];
  $width = $dimensions['width'];
  $file_array['Manifest'] = get_manifest($doc_url, $doc_path_url);
  $file_array['ImageAnnotations'] = get_imageAnnotations($doc_url, $doc_path_url, $height, $width, $title);
  $file_array['NormalSequence'] = get_normal_sequence($doc_url, $doc_path_url, $pid, $height, $width);
  $file_array['AnnotationList'] = get_annotationList($doc_url, $doc_path_url);
  $flat_pid = str_replace(':', '_', $pid);
  $file_path = file_directory_path() . "/$flat_pid/manifest";
  if (!is_dir($file_path)) {
    mkdir($file_path, 0777, TRUE);
  }
  foreach ($file_array as $name => $content) {
    file_put_contents("$file_path/$name.xml", $content);
    $item->add_or_modify_by_reference("$base_url/$file_path/$name.xml", $name, $name, 'text/plain', FALSE, 'Modified by Islandora API');
    file_delete("$file_path/$name.xml");
  }
  rmdir($file_path);
  rmdir(file_directory_path() . "/$flat_pid");
  return "completed";
}

/**
 * returns basic manifest for unpopulated document - ready for test
 *
 * @global url $base_url
 * @param string $pid
 * @return xml
 */
function get_manifest($doc_url, $doc_path_url) {
  $timestamp = date(DATE_W3C);
  module_load_include('inc', 'emic_canvas', 'islandora_rdf_builder');
  $rdf = new IslandoraRdfBuilder();

  $name = "$doc_url/Manifest";
  $children = array();
  $children[] = $rdf->build_child('rdf:type', array('rdf:resource' => 'http://www.openarchives.org/ore/terms/Aggregation'));
  $children[] = $rdf->build_child('rdf:type', array('rdf:resource' => 'http://dms.stanford.edu/ns/Manifest'));
  $children[] = $rdf->build_child('ore:aggregates', array('rdf:resource' => "$doc_url/NormalSequence"));
  $children[] = $rdf->build_child('ore:aggregates', array('rdf:resource' => "$doc_url/AnnotationList"));
  $children[] = $rdf->build_child('ore:aggregates', array('rdf:resource' => "$doc_url/ImageAnnotations"));
  $rdf->addDescriptionNode($name, $children);

  $name = "$doc_path_url/Manifest/manifest.xml";
  $children = array();
  $children[] = $rdf->build_child('ore:describes', array('rdf:resource' => "$doc_url/Manifest"));
  $children[] = $rdf->build_child('dcterms:modified', null, "$timestamp");
  $children[] = $rdf->build_child('dc:format', null, 'application/rdf+xml');
  $children[] = $rdf->build_child('rdf:type', array('rdf:resource' => 'http://www.openarchives.org/ore/terms/ResourceMap'));
 // $children[] = $rdf->build_child('dcterms:creator', array('Emic Shared Canvas'));
  $rdf->addDescriptionNode($name, $children);

  $name = "urn:uuid:07fbb971-6acb-48ce-9ed4-1789dfe5b3a0";
  $children = array();
  $children[] = $rdf->build_child('rdf:type', array('rdf:resource' => 'http://purl.org/dc/terms/Agent'));
  $children[] = $rdf->build_child('foaf:name', null, "SharedCanvas: Python/rdflib");
  $children[] = $rdf->build_child('foaf:mbox', null, "info@islandora.ca");
  $children[] = $rdf->build_child('dc:format', null, 'application/rdf+xml');
  $rdf->addDescriptionNode($name, $children);

  $name = "$doc_url/NormalSequence";
  $children = array();
  $children[] = $rdf->build_child('ore:isDescribedBy', array('rdf:resource' => "$doc_path_url/NormalSequence/NormalSequence.xml"));
  $children[] = $rdf->build_child('rdf:type', array('rdf:resource' => 'http://www.openarchives.org/ore/terms/Aggregation'));
  $children[] = $rdf->build_child('rdf:type', array('rdf:resource' => 'http://dms.stanford.edu/ns/Sequence'));
  $children[] = $rdf->build_child('rdf:type', array('rdf:resource' => 'http://www.w3.org/1999/02/22-rdf-syntax-ns#List'));
  $rdf->addDescriptionNode($name, $children);

  $name = "$doc_path_url/NormalSequence/NormalSequence.xml";
  $children = array();
  $children[] = $rdf->build_child('rdf:type', array('rdf:resource' => 'http://www.openarchives.org/ore/terms/ResourceMap'));
  $children[] = $rdf->build_child('dc:format', null, 'application/rdf+xml');
  $rdf->addDescriptionNode($name, $children);

  $name = "$doc_url/ImageAnnotations";
  $children = array();
  $children[] = $rdf->build_child('ore:isDescribedBy', array('rdf:resource' => "$doc_path_url/ImageAnnotations/ImageAnnotations.xml"));
  $children[] = $rdf->build_child('rdf:type', array('rdf:resource' => 'http://www.openarchives.org/ore/terms/Aggregation'));
  $children[] = $rdf->build_child('rdf:type', array('rdf:resource' => 'http://dms.stanford.edu/ns/ImageAnnotationList'));
  $children[] = $rdf->build_child('rdf:type', array('rdf:resource' => 'http://www.w3.org/1999/02/22-rdf-syntax-ns#List'));
  $rdf->addDescriptionNode($name, $children);

  $name = "$doc_path_url/ImageAnnotations/ImageAnnotations.xml";
  $children = array();
  $children[] = $rdf->build_child('rdf:type', array('rdf:resource' => 'http://www.openarchives.org/ore/terms/ResourceMap'));
  $children[] = $rdf->build_child('dc:format', null, 'application/rdf+xml');
  $rdf->addDescriptionNode($name, $children);
  return $rdf->printRDF();
}

/**
 * Returns a NormalSequence document. 
 * We consider annotate single pages, so the document refrences a single canvas.
 * @param URL $doc_url
 * @return xml
 */
function get_normal_sequence($doc_url, $doc_path_url, $pid, $height, $width) {
  module_load_include('inc', 'emic_canvas', 'islandora_rdf_builder');
  $timestamp = date(DATE_W3C);
  $rdf = new IslandoraRdfBuilder();
  $rdf->root->setAttribute('xmlns:exif', "http://www.w3.org/2003/12/exif/ns#");

  $name = "$doc_url/NormalSequence";
  $children = array();
  $children[] = $rdf->build_child('rdf:first', array('rdf:resource' => "$doc_url/Canvas"));
  $children[] = $rdf->build_child('rdf:rest', array('rdf:resource' => "http://www.w3.org/1999/02/22-rdf-syntax-ns#nil"));
  $children[] = $rdf->build_child('ore:aggregates', array('rdf:resource' => "$doc_url/Canvas"));
  $children[] = $rdf->build_child('rdf:type', array('rdf:resource' => "http://www.w3.org/1999/02/22-rdf-syntax-ns#List"));
  $children[] = $rdf->build_child('rdf:type', array('rdf:resource' => "http://www.openarchives.org/ore/terms/Aggregation"));
  $children[] = $rdf->build_child('rdf:type', array('rdf:resource' => "http://dms.stanford.edu/ns/Sequence"));
  $rdf->addDescriptionNode($name, $children);

  $name = "$doc_path_url/NormalSequence/NormalSequence.xml";
  $children = array();
  $children[] = $rdf->build_child('ore:describes', array('rdf:resource' => "$doc_url/NormalSequence"));
  $children[] = $rdf->build_child('dcterms:modified', null, "$timestamp");
  $children[] = $rdf->build_child('dc:format', null, 'application/rdf+xml');
  $children[] = $rdf->build_child('rdf:type', array('rdf:resource' => 'http://www.openarchives.org/ore/terms/ResourceMap'));
 // $children[] = $rdf->build_child('dcterms:creator', array('rdf:resource' => 'Emic Shared Canvas'));
  $rdf->addDescriptionNode($name, $children);

  $name = "$doc_url/Canvas";
  $children = array();
  $children[] = $rdf->build_child('exif:height', array('rdf:datatype' => "http://www.w3.org/2001/XMLSchema#integer"), $height);
  $children[] = $rdf->build_child('exif:width', array('rdf:datatype' => "http://www.w3.org/2001/XMLSchema#integer"), $width);
  $children[] = $rdf->build_child('dc:title', null, "$pid canvas");
  $children[] = $rdf->build_child('rdf:type', array('rdf:resource' => 'http://dms.stanford.edu/ns/Canvas'));
  $rdf->addDescriptionNode($name, $children);

  $name = "urn:uuid:07fbb971-6acb-48ce-9ed4-1789dfe5b3a0";
  $children = array();
  $children[] = $rdf->build_child('rdf:type', array('rdf:resource' => 'http://purl.org/dc/terms/Agent'));
  $children[] = $rdf->build_child('foaf:name', null, "SharedCanvas: Python/rdflib");
  $children[] = $rdf->build_child('foaf:mbox', null, "info@islandora.ca");
  $children[] = $rdf->build_child('dc:format', null, 'application/rdf+xml');

  return $rdf->printRDF();
}

/**
 * Creates and returns Imageannotation Stream for new Shared Canvas
 * @param url $doc_url
 * @param url $doc_path_url
 * @param integer $height
 * @param integer $width
 * @param string $title
 * @return string
 *
 */
function get_imageAnnotations($doc_url, $doc_path_url, $height, $width, $title) {
  module_load_include('inc', 'emic_canvas', 'islandora_rdf_builder');
  $timestamp = date(DATE_W3C);
  $rdf = new IslandoraRdfBuilder();
  $rdf->root->setAttribute('xmlns:exif', "http://www.w3.org/2003/12/exif/ns#");
  $rdf->root->setAttribute('xmlns:cnt', "http://www.w3.org/2008/content#");
  $rdf->root->setAttribute('xmlns:oac', "http://www.openannotation.org/ns/");
  $option_uuid = generate_uuid();
  $first_resource = generate_uuid();
  $timestamp = date(DATE_W3C);
  $option_uuid = generate_uuid();
  $first_resource = generate_uuid();
  $timestamp = date(DATE_W3C);

  $name = "$doc_url/ImageAnnotations";
  $children = array();
  $children[] = $rdf->build_child('ore:aggregates', array('rdf:resource' => 'urn:uuid:46e74f54-a8e2-41f1-a3a1-24660472c584'));
  $children[] = $rdf->build_child('rdf:first', array('rdf:resource' => 'urn:uuid:b6cf1868-68f9-4994-90dd-240a4a959358'));
//  $children[] = $rdf->build_child('rdf:rest', array('rdf:resource' => 'http://www.w3.org/1999/02/22-rdf-syntax-ns#nil'));
  $children[] = $rdf->build_child('rdf:rest', array('rdf:nodeID' => 'bCfJsWeh546'));
  $children[] = $rdf->build_child('rdf:type', array('rdf:resource' => 'http://www.openarchives.org/ore/terms/Aggregation'));
  $children[] = $rdf->build_child('rdf:type', array('rdf:resource' => 'http://dms.stanford.edu/ns/ImageAnnotationList'));
  $children[] = $rdf->build_child('rdf:type', array('rdf:resource' => 'http://www.w3.org/1999/02/22-rdf-syntax-ns#List'));
// todo add elements for aggregated annotations
  $rdf->addDescriptionNode($name, $children);

  $name = "$doc_path_url/ImageAnnotations/ImageAnnotations.xml";
  $children = array();
  $children[] = $rdf->build_child('dcterms:modified', null, "$timestamp");
  $children[] = $rdf->build_child('dc:format', null, 'application/rdf+xml');
  $children[] = $rdf->build_child('ore:describes', array('rdf:resource' => "$doc_url/ImageAnnotations"));
//  $children[] = $rdf->build_child('dcterms:creator', array('rdf:resource' => 'Emic Shared Canvas'));
  $children[] = $rdf->build_child('rdf:type', array('rdf:resource' => 'http://www.openarchives.org/ore/terms/ResourceMap'));
  $rdf->addDescriptionNode($name, $children);

  $name = "$first_resource";
  $children = array();
  $children[] = $rdf->build_child('rdf:type', array('rdf:resource' => 'http://dms.stanford.edu/ns/ImageAnnotation'));
  $children[] = $rdf->build_child('rdf:type', array('rdf:resource' => 'http://www.openannotation.org/ns/Annotation'));
  $children[] = $rdf->build_child('oac:hasBody', array('rdf:resource' => $option_uuid));
  $children[] = $rdf->build_child('oac:hasTarget', array('rdf:resource' => "$doc_url/Canvas"));
  $rdf->addDescriptionNode($name, $children);

  $name = "urn:uuid:e5c12519-e756-4442-a83d-556848c32714";
  $children = array();
  $children[] = $rdf->build_child('foaf:name', null, "SharedCanvas: Python/rdflib");
  $children[] = $rdf->build_child('foaf:mbox', null, "info@islandora.ca");
  $children[] = $rdf->build_child('rdf:type', array('rdf:resource' => 'http://purl.org/dc/terms/Agent'));
  $rdf->addDescriptionNode($name, $children);

  $name = "$doc_url/JPEG";
  $children = array();
  $children[] = $rdf->build_child('exif:width', array('rdf:datatype' => 'http://www.w3.org/2001/XMLSchema#integer'), $height);
  $children[] = $rdf->build_child('exif:height', array('rdf:datatype' => "http://www.w3.org/2001/XMLSchema#integer"), $width);
  $children[] = $rdf->build_child('rdf:type', array('rdf:resource' => 'http://purl.org/dc/dcmitype/Image'));
  $children[] = $rdf->build_child('rdf:type', array('rdf:resource' => 'http://dms.stanford.edu/ns/ImageBody'));
  $rdf->addDescriptionNode($name, $children);

  $name = "$doc_url/Canvas";
  $children = array();
  $children[] = $rdf->build_child('exif:width', array('rdf:datatype' => 'http://www.w3.org/2001/XMLSchema#integer'), $height);
  $children[] = $rdf->build_child('exif:height', array('rdf:datatype' => "http://www.w3.org/2001/XMLSchema#integer"), $width);
  $children[] = $rdf->build_child('dc:title', null, "$title");
  $children[] = $rdf->build_child('rdf:type', array('rdf:resource' => 'http://dms.stanford.edu/ns/Canvas'));
  $rdf->addDescriptionNode($name, $children);

  $name = "$option_uuid";
  $children = array();
  $children[] = $rdf->build_child('rdf:type', array('rdf:resource' => 'http://dms.stanford.edu/ns/ImageChoice'));
  $children[] = $rdf->build_child('dms:option', array('rdf:resource' => "$doc_url/JPEG"));
  $rdf->addDescriptionNode($name, $children);

  return $rdf->printRDF();
}

/**
 * Creates and returns AnnotationList for new Shared Canvas
 * @param url $doc_url
 * @param url $doc_path_url
 * @return string
 */
function get_annotationList($doc_url, $doc_path_url) {

  module_load_include('inc', 'emic_canvas', 'islandora_rdf_builder');
  $timestamp = date(DATE_W3C);
  $rdf = new IslandoraRdfBuilder();
  $rdf->root->setAttribute('xmlns:oac', "http://www.openannotation.org/ns/");

  $name = "$doc_url/AnnotationList";
  $children = array();
  $children[] = $rdf->build_child('rdf:rest', array('rdf:resource' => "http://www.w3.org/1999/02/22-rdf-syntax-ns#nil"));
  $children[] = $rdf->build_child('rdf:type', array('rdf:resource' => "http://dms.stanford.edu/ns/AnnotationList"));
  $children[] = $rdf->build_child('rdf:type', array('rdf:resource' => "http://www.w3.org/1999/02/22-rdf-syntax-ns#List"));
  $children[] = $rdf->build_child('rdf:type', array('rdf:resource' => "http://www.openarchives.org/ore/terms/Aggregation"));
  $rdf->addDescriptionNode($name, $children);

  $name = "$doc_url/Canvas";
  $children = array();
  $children[] = $rdf->build_child('rdf:type', array('rdf:resource' => "http://dms.stanford.edu/ns/Canvas"));
  $rdf->addDescriptionNode($name, $children);

  $name = "$doc_path_url/AnnotationList/AnnotationList.xml";
  $children = array();
  $children[] = $rdf->build_child('ore:describes', array('rdf:resource' => "$doc_url/AnnotationList"));
//  $children[] = $rdf->build_child('dcterms:creator', array('rdf:resource' => "Emic Shared Canvas"));
  $children[] = $rdf->build_child('rdf:type', array('rdf:resource' => ""));
  $children[] = $rdf->build_child('dc:format', null, 'application/rdf+xml');
  $children[] = $rdf->build_child('dcterms:modified', null, "$timestamp");
  $rdf->addDescriptionNode($name, $children);

  return $rdf->printRDF();
}

/**
 * Parses exif data from supplied jpeg file
 * @param string $file_path
 * path to jpeg file
 * @return array
 * Array has two values - height and width
 */
function get_height_and_width($file_path) {
  $jpeg = file_get_contents($file_path);
  $file = file_directory_path() . '/tmpJPG1.jpg';
  file_put_contents($file, $jpeg);
  exec("exiftool -X $file", $output);
  file_delete(file_directory_path() . '/tmpJPG.jpg');
  $exif = implode('', $output);
  $simplexml = new SimpleXMLElement($exif);
  $width = $simplexml->xpath("/rdf:RDF/rdf:Description/*[namespace-uri()='http://ns.exiftool.ca/File/1.0/' and local-name()='ImageWidth']");
  $width = (string) $width[0];
  $height = $simplexml->xpath("/rdf:RDF/rdf:Description/*[namespace-uri()='http://ns.exiftool.ca/File/1.0/' and local-name()='ImageHeight']");
  $height = (string) $height[0];
  return array(
    'height' => $height,
    'width' => $width
  );
}

/**
 * Echos Fedora streams for javascript retrieval
 * @global url $base_url
 * @param string $pid
 * @param string $stream
 */
function emic_serve($pid, $stream) {
  global $base_url;
  module_load_include('inc', 'fedora_repository', 'api/fedora_item');
  $item = new Fedora_Item($pid);
  $manifest = $item->get_datastream_dissemination($stream);
  echo $manifest;
}

/**
 * Generates uuid
 * @return uuid
 */
function generate_uuid() {
  return sprintf('%04x%04x-%04x-%04x-%04x-%04x%04x%04x',
      mt_rand(0, 0xffff), mt_rand(0, 0xffff),
      mt_rand(0, 0xffff),
      mt_rand(0, 0x0fff) | 0x4000,
      mt_rand(0, 0x3fff) | 0x8000,
      mt_rand(0, 0xffff), mt_rand(0, 0xffff), mt_rand(0, 0xffff)
  );
}