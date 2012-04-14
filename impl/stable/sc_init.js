
var toid = null;
var startDate = 0;

var topinfo = {
  'canvasWidth' : 0,    // scaled width of canvas in pixels
  'numCanvases' : 0,    // number of canvases to display
  'current' : 0,        // current idx in sequence
  'done': [],           // URLs already processed
  'query' : null,       // top level databank
  'sequence' : [],      // Sequence list
  'sequenceInfo' : {},  // uri to [h,w,title]
		
  'annotations' : {
    'image':{},
    'text':{},
    'audio':{},
    'zone':{},
    'comment':{}
  },
  'lists' : {
    'image':{},
    'text':{},
    'audio':{},
    'zone':{},
    'comment':{}
  },
  'raphaels' : {
    'image':{},
    'text':{},
    'audio':{},
    'zone':{},
    'comment':{}
  },
		
  'zOrders' : {
    'image':1,
    'detailImage':1000,
    'text':2000,
    'audio':3000,
    'zone':4000,
    'comment':5000
  },
  'canvasDivHash' : {},
  'builtAnnos' : [],
  'paintedAnnos' : [],
  'audioAnno' : null,
  'waitingXHR' : 0
};


var SVG_NS = "http://www.w3.org/2000/svg";
var XLINK_NS = "http://www.w3.org/1999/xlink";
	
var opts = {
  base:'http://localhost/EmicShared/impl/',
  namespaces: {
    dc:'http://purl.org/dc/elements/1.1/',
    dcterms:'http://purl.org/dc/terms/',
    dctype:'http://purl.org/dc/dcmitype/',
    oac:'http://www.openannotation.org/ns/',
    cnt:'http://www.w3.org/2008/content#',
    dms:'http://dms.stanford.edu/ns/',
    rdf:'http://www.w3.org/1999/02/22-rdf-syntax-ns#',
    ore:'http://www.openarchives.org/ore/terms/',
    exif:'http://www.w3.org/2003/12/exif/ns#'
  }
};


function initCanvas(nCanvas) {
  var w = $('body').width();
  topinfo['origBodyWidth'] = w;
  $('#top_menu_bar').width(w-5);
	
	
  // Make n canvases.  Multiple row logic:
  // 1:  1x1      2: 1x2      3: 1x3
  // 4:  2x2      5: 1x3+1x2  6: 2x3
  // 7:  1x4+1x3  8: 2x4      9: 3x3  (etc)
	
  var rows = Math.floor(Math.sqrt(nCanvas));
  var perrow = Math.ceil(nCanvas/rows);

  var w = w/perrow - (5*perrow);
  var h = $(window).height() - 50;
  h = h/rows;
	
  for (var x=0;x<nCanvas;x++) {
    $('#canvases').append('<div id="canvas_' + x + '" class="canvas"></div>')
    $('#canvas_'+x).width(w);
    $('#canvas_'+x).height(h);
    if (x != 0) {
      if (x % perrow == 0) {
        // below previous first in row
        $('#canvas_'+x).position({
          'of':'#canvas_' + (x-perrow),
          'my':'left top',
          'at':'left bottom',
          'collision':'none',
          'offset': '0 10'
        });
      } else {
        $('#canvas_' +x).position({
          'of':'#canvas_' + (x-1),
          'my':'left top',
          'at':'right top', 
          'collision':'none',
          'offset': '10 0'
        });
      }
    }
  }
  topinfo['canvasWidth'] = w;
  topinfo['numCanvases'] = nCanvas;
	
  if (nCanvas > 2) {
    // Default text off if lots of canvases
    $('#check_show_text').attr('checked',false);
  }
};


function init_ui() {
	
  $('.dragBox').draggable().resizable();
  $('.dragBox').hide();

  $('.dragShade').click(function() {
    var sh = $(this);
    if (sh.text() == '[-]') {
      sh.empty().append('[+]');
      var p = $(this).parent(); // header
      var h = p.height();
      var pp = p.parent(); // box
      var nh = pp.height();
      sh.attr('ph', nh);
      p.next().hide();
      pp.height(h+6);
			
    } else {
      var n = sh.parent().next();
      var nh = sh.attr('ph');
      var p = sh.parent().parent();
      p.height(nh);
      sh.empty().append('[-]');
      n.show();
    }
  });
	
  $('#loadprogress').progressbar({
    value: 2
  }).css({
    height:15,
    width:300,
    opacity: 1.0,
    'z-index': 10000
  });
  $('#loadprogress').position({
    of:'#create_annotation',
    my:'left top',
    at:'right top',
    collision:'none',
    offset:'10 0'
  })
	
  $(".menu_body li:even").addClass("alt");
	
  // Link to menus
  $('.menu_head').click(function () {
    // First find our id, and then use to find our menu
    var id = $(this).attr('id');
    var menubody = $('#' + id + '_body')
    menubody.slideToggle('medium');
    menubody.position({
      'of':'#'+id,
      'my': 'top left',
      'at': 'bottom left',
      'collision':'fit',
      'offset': '0 8'
    })
  });

  try {
    // May not want to allow annotation
    maybe_config_create_annotation();
  } catch (e) {
  // XXX Remove annotation button and shape menu
		
  }
	
  // Hide/Show current page's annos on check
  $('#check_show_text').click(function() {
    if (!$('#check_show_text').is(':checked')) {
      $('.text_anno').hide();
    } else {
      $('.text_anno').show();
    }
  });

  $('#check_show_audio').click(function() {
    if (!$('#check_show_audio').is(':checked')) {
      $('.audio_anno').hide();
    } else {
      $('.audio_anno').show();
    }
  });
	
  $('#check_show_baseImg').click(function() {
    if (!$('#check_show_baseImg').is(':checked')) {
      $('.base_img').hide();
      $('#imgSel').hide();
    } else {
      $('.base_img').show();
      if ($('.imgSelRadio').length > 0 ) {
        $('#imgSel').show();
      }
    }
  });

  $('#check_show_detailImg').click(function() {
    if (!$('#check_show_detailImg').is(':checked')) {
      $('.img_anno').hide();
    } else {
      $('.img_anno').show();
    }
  });
	
  $('#check_show_comment').click(function() {
    if (!$('#check_show_comment').is(':checked')) {
      $('#comment_annos').hide();
    } else {
      if ($('.comment_title').length > 0 ) {
        $('#comment_annos').show();
      }
    }
  });
	
  $('#check_view_imgSel').click(function() {
    if (!$('#check_view_imgSel').is(':checked')) {
      $('#imgSel').hide();
    } else {
      if ($('.imgSelRadio').length > 0) {
        $('#imgSel').show();
      }
    }
  });

  $('#check_view_uri').click(function() {
    if (!$('#check_view_uri').is(':checked')) {
      $('.canvasUri').hide();
    } else {
      $('.canvasUri').show();
    }
  });
	
  $('#check_view_zpr').click(function() {
    if (!$('#check_view_zpr').is(':checked')) {
      $('.zoomStart').hide();
    } else {
      $('.zoomStart').show();
    }
  });
	
  // Make show menu sortable to manually tweak Z levels
  $('#show_body').sortable({
    tolerance:'pointer',
    items:'.show_sort',
    stop:reorder_layers
  });
	
  // Initialize audio player up front, even if not used
  // Better than re-building every canvas
  $("#jquery_jplayer_1").jPlayer({
    ready: function () {
    //$(this).bind($.jPlayer.event.timeupdate, on_audio_currentTime);
    },
    // preload: 'auto',
    volume: 0.8,
    swfPath: "/js",
    errorAlerts: true,
    warningAlerts: true,
    supplied: "mp3"   // XXX: dynamic? Recreate jplayer to change?
  });
	
  $("#slider_volume").slider({
    value: 80,
    orientation: "horizontal",
    range: "min",
    animate: true,
    change: function(e, ui) {
      var jp = $('#jquery_player_1');
      var ratio = ui.value / 100;
      try {
        jp.jPlayer('volume', ratio);
      } catch (e) {};
    }
  });
	
  $('#slider_folios').slider({
    value: 0,
    orientation: "horizontal",
    range: "min",
    animate: true,
    slide: function(e, ui) {
      var nf = Math.max(1, Math.floor(ui.value / 100 * 16));
      $('#viewNumCanvas').empty();
      $('#viewNumCanvas').append(nf);
    },
    change: function(e, ui) {
      var nf = Math.max(1, Math.floor(ui.value / 100 * 16));
      if (topinfo['numCanvases'] != nf) {
        topinfo['uriParams']['n'] = nf;
        topinfo['numCanvases'] = nf;
        var hsh = makeUriHash();
        $(location).attr('hash',hsh);
        initCanvas(topinfo['numCanvases']);
        showPages();
      }
    }
  });
	
  // Enable iPad swipe navigation
  var swipeLeft = function() {
    nextPage();
  }
  var swipeRight = function() {
    prevPage();
  }
  var swOpts = {
    'swipeLeft':swipeLeft,
    'swipeRight':swipeRight,
    threshold:20
  };
  $(function(){
    $('body').swipe(swOpts)
  });
	
  // Refresh Canvas if browser is resized
  // We're called as per move... so need wait till finished resizing
  $(window).resize(function() {
    var w = $('body').width();
    topinfo['bodyWidth'] = w;
    if (toid != null) {
      // Be considerate and clear previous timeout
      window.clearTimeout(toid)
    }
    toid = window.setTimeout(maybeResize, 1000)
  });
}

function maybeResize() {
  var w = $('body').width();
  // Allow for slight tweak on size from original for scrollbars
  if (w == topinfo['bodyWidth'] && Math.abs(topinfo['origBodyWidth']-w) > 20) {
    // We've been stationary for 1 second
    toid = null;
    var b = topinfo['origBodyWidth'];
    topinfo['bodyWidth'] = 0;
    if (w != b) {
      initCanvas(topinfo['numCanvases']);
      showPages();
    }
  }
}


// Let's start it up!

$(document).ready(function(){
  // RDF Initialization
  var rdfbase = $.rdf(opts);
  topinfo['query'] = rdfbase;

//  for(var prop in rdfbase) {
//    if(rdfbase.hasOwnProperty(prop))
//      alert(prop + " = " + rdfbase[prop]);
//  }


  var l = $(location).attr('hash');
  var uriparams = {};
  var nCanvas = 1;
  var start = 0;
  if (l[0] == '#' && l[1] == '!') {
    // Process initialization
    var params = l.substr(2,l.length).split('&');
    for (var p=0,prm;prm=params[p];p++) {
      var tup = prm.split('=');
      var key = tup[0];
      var val = tup[1];
      if (key == 's') {
        start = parseInt(val);
        uriparams['s'] = start;
      } else if (key == 'n') {
        nCanvas = parseInt(val);
        uriparams['n'] = nCanvas;
      }
    }
  }
  topinfo['uriParams'] = uriparams
	
  // Initialize UI
  init_ui();
  // Setup a basic Canvas with explicit width to scale to from browser width
  initCanvas(nCanvas)
	
  // Manifest Initialization
  var manuri = $('#manifest').attr('href');
  if (manuri != undefined) {
    fetchTriples(manuri, rdfbase, cb_process_manifest);
  } else {
    repouri = $('#repository').attr('href');
    fetchTriples(repouri, rdfbase, cb_process_repository);
  }

});
