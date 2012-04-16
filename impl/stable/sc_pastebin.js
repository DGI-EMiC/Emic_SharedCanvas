
function init_pb() {
	$('#create_body').append('<li id="pasteBinSel">&nbsp; Public: <span style="float:right"><input type="radio" name="blog_radio" id="pb_pastebin"></span></span></li>');
	$("#create_body li:even").addClass("alt");	
}

function pb_postData(title, data) {
  
    data = escape(data);
	$.ajax({
		type:'POST',
                url:emic_params.islandora_post_url,
		data: {title:title, data:data},
		success: function(data,status,xhr) {pb_getPaste(data);},
		error: function(data,status,xhr) {alert('Failed to post')}	
	});	
}

function pb_getList() {
 
	$.ajax({
		type:'GET',
               
		url: emic_params.get_annotation_list_url,
		success: function(data,status,xhr) {

			var l = $.parseJSON(data);
			
			for (var i=0,info;info=l[i];i++){
				var pid = info;
				
				
				$('#canvases .canvas').each(function() {
					var cnv = $(this).attr('canvas');
					
						pb_getPaste(pid);						
					
				});
			}
		},
		error: function(data,status,xhr) {alert('Failed to retrieve List')}	
	});		
}

function pb_getPaste(pid) {
  
	$.ajax({
		type:'GET',
		url: emic_params.islandora_get_annotation +pid,
		success: function(data,status,xhr) {
			load_commentAnno(data);},
		error: function(data,status,xhr) {alert(data)}
	});		
}
