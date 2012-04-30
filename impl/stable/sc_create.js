

// XXX Need author info in annotations
// Plus other XXXs (!)

// Raphael overwrites CSS with defaults :(
var outsideStyle = {
  fill: '#FFBBFF',
  opacity: 0.7,
  'stroke-width': 2,
  stroke: 'black'
};
var insideStyle = {
  fill: '#FFFFFF',
  opacity: 0.3,
  'stroke-width': 2,
  stroke: 'black',
  'stroke-dasharray': '- '
};

function fetch_comment_annotations() {
  pb_getList();
	
}

function maybe_config_create_annotation() {
		
  $('#create_annotation').click(startAnnotating);
  $('.diabutton').button();
  $('#cancelAnno').click(closeAndEndAnnotating);
  $('#saveAnno').click(saveAndEndAnnotating);

  $('.annoShape').click(function() {
    var typ = $(this).attr('id').substr(10,5);
    topinfo['svgAnnoShape'] = typ;
    $('.annoShape').css('border', '0px');
    $(this).css('border', '1px solid black');
  });
	
  var shp = $('.annoShape').filter(':first');
  shp.css('border', '1px solid black');
  topinfo['svgAnnoShape'] = shp.attr('id').substr(10,5);
	
  // Install PasteBin
  init_pb();

}

function startAnnotating() {
	
  if ($('#create_annotation').text() == 'Annotating') {
    return;
  }
	
  $('#create_annotation').css({
    color:'#808080'
  });
  $('#create_annotation').empty().append('Annotating');
  $('#create_annotation_box').show();
  $('#create_annotation_box').offset({
    top:200,
    left:35
  });
	
  $('#canvases .canvas').each(function() {
    var cnv = $(this).attr('canvas');
    initForCreate(cnv);
  });
}

function saveAndEndAnnotating() {
	
  // check we have a service selected
  var which = $('#create_body input[name="blog_radio"]:radio:checked').attr('id');
  if (which == undefined) {
    alert('You must select a service to save the Annotation to, from the list to the left.');
    return;
  }
	
  var okay = saveAnnotation();
  if (okay) {
    closeAndEndAnnotating();
  }
}

function closeAndEndAnnotating() {

  $('#create_annotation').empty().append('Annotate');
  $('#create_annotation').css({
    color:'#000000'
  });
  $('#canvases .canvas').each(function() {
    cnv = $(this).attr('canvas');
    destroyAll(cnv);
  });
	
  $('#create_annotation_box').hide();
  // empty fields
  $('#anno_title').val('');
  $('#anno_text').val('');
  $('#anno_aboutCanvas').prop('checked', false);
  $('#anno_isResource').prop('checked', false);
    
}
	
//We do creation by trapping clicks in an invisible SVG box
//This way we have the dimensions without messing around 
//converting between page clicks and canvas clicks

function initForCreate(canvas) {
	
  var r = mk_raphael('comment', canvas, topinfo['canvasDivHash'][canvas])
  var invScale = 1.0 / r.newScale;
  var ch = Math.floor(r.height * invScale);
  var cw = Math.floor(r.width * invScale);
  var prt = r.wrapperElem;
	
  // Ensure we're above all painting annos
  $(prt).css('z-index', 5000);
	
  var bg = r.rect(0,0,cw,ch);
  bg.attr({
    'fill': 'white',
    'opacity': 0.15
  });
  // bg.toBack();
  bg.creating = null;
  bg.invScale = invScale;
  bg.myPaper = r;
  bg.myShapes = [];
  r.annotateRect = bg;
	
  bg.drag(function(dx,dy) {
    this.creating.resizeFn(dx, dy)
  }, switchDown, switchUp);
	
}

function destroyAll(canvas) {
  if ( topinfo['raphaels']['comment'][canvas]){
    var r = topinfo['raphaels']['comment'][canvas];
    var bg = r.annotateRect;
    for (var x in bg.myShapes) {
      var sh = bg.myShapes[x];
      if (sh.set != undefined) {
        sh.set.remove();
      } else {
        sh.remove();
      }
    };
    bg.remove();
    $(r.wrapperElem).remove();
    $(r).remove();
  }
  topinfo['raphaels']['comment'][canvas] = undefined;
  pb_getList();
}

function saveAnnotation() {
  // Basic Sanity Checks
  var title = $('#anno_title').val();
  var content = $('#anno_text').val();
  var typ = $('#anno_type :selected')[0].value;
  var isResc = $('#anno_isResource').prop('checked');
  var tgtsCanvas = $('#anno_aboutCanvas').prop('checked');
	
  if (!content || (!title && typ == 'comment')) {
    alert('An annotation needs both title and content');
    return 0;
  } else if (isResc && content.substr(0,4) != 'http') {
    // check content is vaguely like a URI
    alert('The content of the annotation must be an HTTP uri if "Text is Link" is selected.');
    return 0;
  } else if (!isResc && (typ == 'image' || typ == 'audio')) {
    alert('Image or Audio annotations must have the "Text is URL" box checked');
    return 0;
  }
	
  // Create
  var rinfo = create_rdfAnno();
  var rdfa = rinfo[0];
  var tgt = rinfo[1];
	
  if (tgt == null) {
    alert('You must either check "About Canvas" or draw a shape around the target.');
    return 0;
  }
	
  var which = $('#create_body input[name="blog_radio"]:radio:checked').attr('id');
  // Save
  if (which == 'pb_pastebin') {
    var data = $(rdfa).rdf().databank.dump({
      format:'text/turtle',
      serialize:true
    });
    // var data = $(rdfa).rdf().databank.dump({format:'application/rdf+xml',serialize:true});
    pb_postData(tgt, rdfa);
  } else {
    postToBlog(title, rdfa, typ);
  }
  return 1;
}

function nodeToXml(what) {
  // MUST use 's as in attribute on span
  var xml = '<svg:' + what.nodeName + " xmlns:svg='" + SVG_NS +"' ";
  for (a in what.attributes) {
    var attr = what.attributes[a];
    if (attr.nodeName != undefined) {
      xml += (attr.nodeName + "='"+attr.nodeValue+"' ");
    }
  }
  xml += ("></svg:" + what.nodeName+'>');
  return xml;
}


function create_rdfAnno() {

  var nss = opts.namespaces;
  var typ = null;
  $('#anno_type :selected').each(function() {
    typ = this.value;
  });

  if (typ == 'comment') {
    var clss = 'oac:Annotation';
    var fullclss = nss['oac'] +'Annotation';
  } else if (typ == 'transcription' || typ == 'initial' || typ == 'rubric') {
    var clss = 'dms:TextAnnotation';
    var fullclss = nss['dms'] + 'TextAnnotation';
  } else if (typ == 'image') {
    var clss = 'dms:ImageAnnotation';
    var fullclss = nss['dms']+'ImageAnnotation';
  } else if (typ == 'audio') {
    var clss = 'dms:AudioAnnotation';
    var fullclss = nss['dms']+'AudioAnnotation';
  }
	
  if (typ == 'initial') {
    var bodyClass = 'dms:InitialBody';
    var bodyFullClass = nss['dms']+'InitialBody';
  } else if (typ == 'rubric') {
    var bodyClass = 'dms:RubricBody';
    var bodyFullClass = nss['dms']+'RubricBody';
  } else {
    var bodyClass = null;
    var bodyFullClass = null;
  }
		
  var now = isodate(new Date());
  // Generate namespaces from RDF options
  var xmlns = '';
  for (x in nss) {
    xmlns += ('xmlns:'+x+'="'+nss[x]+'" ')
  }

  var rdfa = '<div '+xmlns+'>'; // Start wrapper for XMLNS

  // Build Annotation
  var annoUU = new UUID();
  rdfa += '<div about="urn:uuid:' + annoUU + '"> '; // Start Anno
  rdfa += ('<a rel="rdf:type" href="'+fullclss+'"></a>');
  rdfa += '<span property="dcterms:created" content="' + now + '"></span> ';

  var title = $('#anno_title').val();
  if (title != '') {
    rdfa += '<span property="dc:title" content="' + title + '"></span>';
  }
    
  try {
    // XXX Gdata specific, but can send to other services
    var which = $('#create_body input[name="blog_radio"]:radio:checked').attr('id');
    var bFO = topinfo['blogs'][which][2];
    var authors = bFO.author;
    for (var a=0, auth; auth=authors[a]; a++) {
      var email = auth.email.getValue(); // check for noreply@blogger.com
      var name = auth.name.getValue();
      var uri = auth.uri.getValue();
      rdfa += '<a rel="dcterms:creator" href="' + uri + '"></a> ';
      rdfa += '<div about="' + uri + '"> ';
      rdfa += '<a rel="rdf:type" href="http://xmlns.com/foaf/0.1/Agent"></a>';
      rdfa += '<span property="foaf:name" content="' + name + '"></span> ';
      if (email != 'noreply@blogger.com') {
        rdfa += '<span property="foaf:mbox" content="' + email + '"></span> ';
      }
      rdfa += '</div> '; // Close Creator
    }
	
    var rights = bFO.getRights();
    if (rights != undefined) {
      var rtxt = rights.getText();
      var ruri = rights.getUri();
      if (rtxt) {
        rdfa += '<span property="dc:rights" content="' + rtxt + '"></span> ';
      } else {
        rdfa += '<a rel="dcterms:rights" href="' + ruri + '"></a> ';
      }
    }
  } catch (e) {};
    
  // Build Body
  var isResc = $('#anno_isResource').prop('checked');
  var tgtsCanvas = $('#anno_aboutCanvas').prop('checked');
  var content = $('#anno_text').val();
    
  if (isResc == true) {
    // XXX Could be constrained resource, eg part of an XML or image
    // So would need to build constraint ... too hard!
    rdfa += '<b>See:</b> <a rel="oac:hasBody" href="' + content + '">' + content + '</a>';
  } else {
    var contUU = new UUID();
    rdfa += '<a rel="oac:hasBody" href="urn:uuid:' + contUU +'"></a> ';
    rdfa += '<div about="urn:uuid:' + contUU + '"> ';
    rdfa += '<a rel="rdf:type" href="http://www.w3.org/2008/content#ContentAsText"></a> ';
    if (bodyFullClass != null) {
      rdfa += '<a rel="rdf:type" href="'+bodyFullClass+'"></a>';
    }
    rdfa += '<span property="cnt:chars">' + content + '</span> ';
    rdfa += '<span property="cnt:characterEncoding" content="utf-8"></span>';
    rdfa += '</div> ';  // Close Body
  }
    
  // Build Target(s)
  // For each Canvas, for each SVG, build a ConstrainedTarget and SvgConstraint
  // XXX Bad assumption that about ALL canvases if tgtsCanvas
  // XXX Should allow selection of other Annotations (eg Texts)
    
  var target = null;
  $('#canvases .canvas').each(function() {
    var cnv = $(this).attr('canvas');
    // var cnv = emic_canvas_params.object_base +'/Canvas';
    if(cnv){
      if (tgtsCanvas == true) {
        target = cnv;
        rdfa += '<a rel="oac:hasTarget" href="' + cnv +'"></a>';
      } else {
        var r = topinfo['raphaels']['comment'][cnv];
        var bg = r.annotateRect;
        var stuff = bg.myShapes;
        for (s in stuff) {
          target = cnv;
          var svgxml = nodeToXml(stuff[s].node);
          svgxml = svgxml.replace('<', '&lt;');
          svgxml = svgxml.replace('<', '&lt;');
          svgxml = svgxml.replace('>', '&gt;');
          svgxml = svgxml.replace('>', '&gt;');
          var ctuu = new UUID();
          rdfa += '<a rel="oac:hasTarget" href="urn:uuid:' + ctuu +'"></a>';
          rdfa += '<div about="urn:uuid:' + ctuu +'">';
          rdfa += '<a rel="rdf:type" href="http://www.openannotation.org/ns/ConstrainedTarget"></a>';
          rdfa += '<a rel="oac:constrains" href="' + cnv + '"></a>';
          var svguu = new UUID();
          rdfa += '<a rel="oac:constrainedBy" href="urn:uuid:' + svguu + '"></a>'
          rdfa += '<div about="urn:uuid:' + svguu + '">';
          rdfa += '<a rel="rdf:type" href="http://www.openannotation.org/ns/SvgConstraint"></a>';
          rdfa += '<a rel="rdf:type" href="http://www.w3.org/2008/content#ContentAsText"></a>';
          rdfa += '<span property="cnt:chars" content="' + svgxml + '"></span>';
          rdfa += '<span property="cnt:characterEncoding" content="utf-8"></span>';
          rdfa += "</div>"; // Close Constraint
          rdfa += "</div>"; // Close Constrained Target
        }
      }
    }
  });
  rdfa += "</div>"; // Close Annotation
  rdfa += "</div>"; // Close wrapper
  return [rdfa, target];
}

switchDown = function(x,y) {
  var fixedxy = fixXY(this,x,y);
  var x = fixedxy[0];
  var y = fixedxy[1];
  var which = topinfo['svgAnnoShape'];
	
  if (which == 'circ') {
    this.creating = mkCircle(this,x,y);
    this.myShapes.push(this.creating);
  } else if (which == 'rect') {
    this.creating = mkRect(this, x,y);
    this.myShapes.push(this.creating);
  } else {
    if (this.creating == null) {
      this.creating = mkPoly(this,x,y);
      this.myShapes.push(this.creating);
    } else {
      this.creating.addPoint(this,x,y);
    }
  }
}

switchUp = function(x, y) {
  var which = topinfo['svgAnnoShape'];
  if (which == 'circ') {
    this.creating.start=[];
    this.creating = null;
  } else if (which == 'rect') {
    this.creating.set.start=[];
    this.creating = null;
  }
}

function fixXY(what, x, y) {
  // modify for x,y of wrapper
  var r = what.myPaper;
  var wrap = r.wrapperElem;

  // This is location of canvas
  var offsetLeft = $(wrap).offset().left;
  var offsetTop = $(wrap).offset().top;
  y-= offsetTop;
  x -= offsetLeft;
	
  // And for scroll in window
  var pageOffsetTop = $('body').scrollTop();
  var pageOffsetLeft = $('body').scrollLeft();
  y += pageOffsetTop;
  x += pageOffsetLeft;
	
  // And now scale for Canvas resizing
  x = Math.floor(x * what.invScale);
  y = Math.floor(y * what.invScale);
  return [x,y]
}



function mkGrabber(what,poly,x,y,idx) {
  var myr = Math.floor(10*what.invScale);
  var r = what.myPaper;
  var c = r.circle(x,y,myr);
  c.attr(insideStyle);
  c.pointIdx = idx;
  c.poly = poly;
  poly.set.push(c);
  c.start = [x,y];
	
  var mdf = function() {
    this.moved = 0;
    this.start = [this.attr("cx"), this.attr("cy")]
  };

  var muf = function() {
    if (what.creating == this.poly && this.pointIdx == 0) {
      what.creating = null;
      this.poly.attr('path', this.poly.attr('path') + 'Z');
    } else if (!this.moved) {
      // delete point
      pth = Raphael.parsePathString(this.poly.attr("path"));
      pth.splice(this.pointIdx, 1);
      this.poly.attr("path", pth);
      // Now shuffle down all subsequent points
      for (var i = this.pointIdx, pt; pt = this.poly.set[i+1]; i++) {
        pt.pointIdx -= 1;
      }
      this.remove();
    }
    this.start = undefined;
  };

  var move = function (dx, dy) {
    dx = Math.floor(dx * what.invScale);
    dy = Math.floor(dy * what.invScale);
		
    this.attr({
      cx: this.start[0] + dx,
      cy: this.start[1] + dy
    })
    var pathsplit = Raphael.parsePathString(this.poly.attr("path"))
    pathsplit[this.pointIdx][1] = Math.floor(this.start[0]+dx);
    pathsplit[this.pointIdx][2] = Math.floor(this.start[1]+dy);

    this.poly.attr('path', pathsplit);
    this.moved = 1;
  };
  c.moveFn = move;
  c.drag(move, mdf, muf);
  return c;
}

function mkPoly(what, x,y) {

  addPointFn = function(what, x,y) {
    this.attr('path', this.attr('path') + ('L'+x+','+y) );
    c = mkGrabber(what, this, x,y,this.attr('path').length-1);
  };

  var mdf = function() {
    this.set.tmp = [0,0];
  };
  var muf = function() {
    this.set.tmp = undefined;
  };
  var move = function(dx,dy) {
    dx=Math.floor(dx * what.invScale);
    dy=Math.floor(dy * what.invScale);
    this.set.translate(dx-this.set.tmp[0], dy-this.set.tmp[1]);
    this.set.tmp = [dx,dy];
  };
  var resizefn = function(dx,dy) {
    dx=Math.floor(dx * what.invScale);
    dy=Math.floor(dy * what.invScale);
    c = this.set[this.set.length-1];
    c.moveFn(dx,dy);
  };
	
  var r = what.myPaper;
  var s = r.set();
  var outer = r.path("M" +x + ',' + y);
  // $(outer.node).addClass('outsideSvg')
  outer.attr(outsideStyle);
  outer.attr()
  outer.addPoint = addPointFn;
  s.push(outer)
  outer.set = s;
	
  c = mkGrabber(what,outer,x,y,0);
  outer.drag(move, mdf, muf);
  outer.resizeFn = resizefn;
  outer.dblclick(function() {
    this.set.remove();
  });
  return outer;
}


function mkCircle(what, x,y) {

  innerSize = Math.floor(10 * what.invScale);
	            	
  mdf = function() {
    this.start = [this.attr("cx"), this.attr("cy"), this.attr('r')]
  };
	
  muf = function() {
    this.start = undefined;
  };
	
  move = function (dx, dy) {
    this.set.attr(
    {
      cx: this.start[0] + Math.floor(dx * what.invScale),
      cy: this.start[1] + Math.floor(dy * what.invScale)
    });
  };
	
  resize = function(dx, dy) {
    this.attr('r', this.start[2] + Math.floor(dx * what.invScale));
    this.inner.attr('r', this.start[2] + (Math.floor(dx * what.invScale) - innerSize));
  };
	
  var r = what.myPaper;
  var st = r.set();
  var outer = r.circle(x,y,innerSize);
  // $(outer.node).addClass('outsideSvg')
  outer.attr(outsideStyle);
  outer.start = [x,y,innerSize];
  outer.set = st;
	
  var inner = r.circle(x, y, 0);
  // $(inner.node).addClass('insideSvg');
  inner.attr(insideStyle);
  inner.toFront();
  inner.set = st;
	
  st.push(outer);
  st.push(inner);
  outer.inner = inner;
	
  inner.drag(move, mdf, muf);
  outer.drag(resize, mdf, muf);
  outer.resizeFn = resize;
  inner.dblclick(function() {
    this.set.remove();
  });
  return outer;
}


function mkRect(what, x,y) {
	            
  innerSize = Math.floor(14 * what.invScale);
	
  mdf = function() {
    this.set.start = [
    Math.floor(this.set.outer.attr('x') ),
    Math.floor(this.set.outer.attr('y') ),
    Math.floor(this.set.outer.attr('height') ),
    Math.floor(this.set.outer.attr('width') )
    ];
  };
	
  muf = function() {
    this.set.start = undefined;
  };
	
  move = function(dx, dy) {
    this.set.outer.attr(
    {
      'x': this.set.start[0] + Math.floor(dx * what.invScale),
      'y' : this.set.start[1] + Math.floor(dy * what.invScale)
    });
    this.set.inner.attr(
    {
      'x': this.set.start[0] + this.set.start[3] + Math.floor(dx * what.invScale) - innerSize,
      'y' : this.set.start[1] + this.set.start[2] + Math.floor(dy * what.invScale) - innerSize
    });
  };
							 
  resize = function(dx, dy) {
    this.set.outer.attr(
    {
      'height' : this.set.start[2] + Math.floor(dy * what.invScale),
      'width' : this.set.start[3] + Math.floor(dx * what.invScale)
    });
    this.set.inner.attr(
    {
      'x' : this.set.start[0] + this.set.start[3] + Math.floor(dx * what.invScale) - innerSize,
      'y': this.set.start[1] + this.set.start[2] + Math.floor(dy * what.invScale) - innerSize
    });
  };
	
  var r = what.myPaper;
  var st = r.set();
  st.start = [x, y, innerSize,innerSize];

  var outer = r.rect(x,y, innerSize,innerSize);
  outer.attr(outsideStyle);
  // $(outer.node).addClass('outsideSvg')
  outer.set = st;
	
  var inner = r.rect(x,y, innerSize+1,innerSize+1);
  inner.attr(insideStyle);
  // $(inner.node).addClass('insideSvg')
  inner.toFront();
  inner.set = st;

  st.push(outer);
  st.push(inner);
  st.outer = outer;
  st.inner = inner;
	
  inner.drag(resize, mdf, muf);
  outer.drag(move, mdf, muf);
  outer.resizeFn = resize;
  outer.dblclick(function() {
    this.set.remove();
  });
  return outer;
}
