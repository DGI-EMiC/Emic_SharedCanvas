<?php

/**
 * Builds initial suite of xml's for Shared Canvas installation
 *
 * @global url $base_url
 * @param string $pid
 *
 */

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
  $children[] = $rdf->build_child('foaf:name', null, "SharedCanvas: Islandora/rdflib");
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
  $rdf->root->setAttribute('xmlns:oa', "http://www.w3.org/ns/openannotation/core/");
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
  $children[] = $rdf->build_child('rdf:type', array('rdf:resource' => 'http://www.w3.org/ns/openannotation/core/Annotation'));
  $children[] = $rdf->build_child('oa:hasBody', array('rdf:resource' => $option_uuid));
  $children[] = $rdf->build_child('oa:hasTarget', array('rdf:resource' => "$doc_url/Canvas"));
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
  $rdf->root->setAttribute('xmlns:oa', "http://www.w3.org/ns/openannotation/core/");

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
  $children[] = $rdf->build_child('rdf:type', array('rdf:resource' => ""));
  $children[] = $rdf->build_child('dc:format', null, 'application/rdf+xml');
  $children[] = $rdf->build_child('dcterms:modified', null, "$timestamp");
  $rdf->addDescriptionNode($name, $children);

  return $rdf->printRDF();
}

/**
 * Retrieves exif data from supplied image
 * @param pid $pid
 * @return array
 * Array has two values - height and width
 */
function get_height_and_width($pid) {
  module_load_include('inc', 'fedora_repository', 'api/fedora_item');
  $item = new Fedora_Item($pid);
  if (array_key_exists('TECHMD', $item->datastreams)) {

    $content = $item->get_datastream_dissemination('TECHMD');
    $sxml = simplexml_load_string($content);
    $sxml->registerXPathNamespace('fits', "http://hul.harvard.edu/ois/xml/ns/fits/fits_output");
    $image_height = $sxml->xpath('//fits:imageHeight');
    $image_height = (string) $image_height[0];
    $image_width = $sxml->xpath('//fits:imageWidth');
    $image_width = (string) $image_width[0];
    $height_width_arr = array(
      'height' => $image_height,
      'width' => $image_width,
    );
    return $height_width_arr;
  }
  global $base_url;
  $filename = $base_url . "/fedora/repository/$pid/JPEG";
  $flat_pid = str_replace(':', '_', $pid);
  $storage_file = file_directory_path() . "/$flat_pid.jpg";
  $image = $item->get_datastream_dissemination("JPEG");
  file_put_contents($storage_file, $image);
  $dimensions = getimagesize($storage_file);
  unlink($storage_file);
  $height_width_arr = array(
    'height' => $dimensions[1],
    'width' => $dimensions[0],
  );
  return $height_width_arr;
}

/**
 * Echos Fedora streams for javascript retrieval
 * @global url $base_url
 * @param string $pid
 * @param string $stream
 */
function islandora_serve($pid, $stream) {
  global $base_url;
  $doc_url = "$base_url/fedora/repository/$pid";
  $doc_path_url = "$base_url/emic/serve/$pid";

  if ($stream == 'Manifest') {
    $manifest = get_manifest($doc_url, $doc_path_url);
    echo $manifest;
    return;
  }

  $dimensions = get_height_and_width($pid);
  $height = $dimensions['height'];
  $width = $dimensions['width'];

  if ($stream == 'NormalSequence') {
    $sequence = get_normal_sequence($doc_url, $doc_path_url, $pid, $height, $width);
    echo $sequence;
    return;
  }
  if ($stream == 'ImageAnnotations') {
    $imageAnnotations = get_imageAnnotations($doc_url, $doc_path_url, $height, $width, $title);
    echo $imageAnnotations;
    return;
  }
}

/**
 * Generates uuid
 * @return uuid
 */
function generate_uuid() {
  return sprintf('%04x%04x-%04x-%04x-%04x-%04x%04x%04x', mt_rand(0, 0xffff), mt_rand(0, 0xffff), mt_rand(0, 0xffff), mt_rand(0, 0x0fff) | 0x4000, mt_rand(0, 0x3fff) | 0x8000, mt_rand(0, 0xffff), mt_rand(0, 0xffff), mt_rand(0, 0xffff)
  );
}