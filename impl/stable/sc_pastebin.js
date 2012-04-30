
function init_pb() {
  $('#create_body').append('<li id="pasteBinSel">&nbsp; Public: <span style="float:right"><input type="radio" name="blog_radio" checked = "checked" id="pb_pastebin"></span></span></li>');
  $("#create_body li:even").addClass("alt").hide();

}

function pb_postData(title, data) {
  data = escape(data);
  $.ajax({
    type:'POST',
    url:emic_canvas_params.islandora_post_url,
    data: {
      title:title,
      data:data
    },
    success: function(data,status,xhr) {
      pb_getPaste(data);
    },
    error: function(data,status,xhr) {
      alert('Failed to post')
    }
  });
}

function pb_getList() {
  $.ajax({
    type:'GET',
               
    url: emic_canvas_params.get_annotation_list_url,
    success: function(data,status,xhr) {

      var l = $.parseJSON(data);
      if( l != null){
        for (var i=0,info;info=l[i];i++){
          var pid = info;
				
				
          $('#canvases .canvas').each(function() {
            var cnv = $(this).attr('canvas');
					
            pb_getPaste(pid);
					
          });
        }
      }
    },
    error: function(data,status,xhr) {
      alert('Failed to retrieve List')
    }
  });
}

function pb_getPaste(pid) {
  
  $.ajax({
    type:'GET',
    url: emic_canvas_params.islandora_get_annotation +pid,
    success: function(data,status,xhr) {
      load_commentAnno(data);
    },
    error: function(data,status,xhr) {
      console.dir(data)
    }
  });
}

// unimplemented.

function pb_deleteAnno(urn) {

  var selector = '#anno_'+urn;
  var classSelector = '.svg_'+urn;
  $.ajax({
    type:'POST',
    url:emic_canvas_params.islandora_delete_annotation + urn,
    data: urn,
    success: function(data,status,xhr) {
      $(selector).next().remove();
      $(selector).remove();
      $(classSelector).remove();
    },
    error: function(data,status,xhr) {
      alert('Failed to delete annotation')
    }
  });
}