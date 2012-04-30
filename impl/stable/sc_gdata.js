
// Initialize GData for Blogger annotations

try {
    google.load('gdata', '1.x', {packages: ['blogger']});
    google.setOnLoadCallback(checkGdataToken);
} catch(e) {}

var gdataSvc = undefined;
var gdataScope = "http://www.blogger.com/feeds";
topinfo['blogs'] = {};
	
function setupGdataSvc() {
    token = google.accounts.user.login(gdataScope);
    if (token) {
		$('#blog_loginout').remove();    	
    	uri = "http://www.blogger.com/feeds/default/blogs";
        gdataSvc.getBlogFeed(uri, handleListBlogs, log);
    }       
}

function add_blogger_button() {
	$('#create_body').append('<li id="blog_loginout">&nbsp; Blogger: <span style="float:right"><button onclick="setupGdataSvc();" value="login">login</button><button onclick=\'window.location="https://www.google.com/accounts/NewAccount?hl=en&continue=http://www.blogger.com/create-blog.g&service=blogger&naui=8"\'>new</button></span></li>').hide();
	$("#create_body li:even").addClass("alt");
}
	

function checkGdataToken() {
	gdataSvc = new google.gdata.blogger.BloggerService('oac-anno-djatoka-1');
	tok = google.accounts.user.checkLogin(gdataScope);
	if (tok) {
		$('#blog_loginout').remove();
        uri = "http://www.blogger.com/feeds/default/blogs";
        gdataSvc.getBlogFeed(uri, handleListBlogs, log);
	} else {
		// add login button
		add_blogger_button();
	}
}

function logout() {
    if (google.accounts.user.checkLogin(gdataScope)) {
            google.accounts.user.logout();
            $('#blog_loginout').remove();
            $('.blog_select_li').remove();
            // Add login button back
            add_blogger_button();
    }
}

function handleListBlogs(resp) {
    var blogs = resp.feed.getEntries();
    if (!blogs.length) {
           	// No blogs.  Request that user create one
    		$('#create_body').append("<li>No blogs found. Please create one.</li>");
    } else {
        $('#create_body').append('<li id="blog_loginout">&nbsp; Blogger: <span style="float:right"><button onclick="logout()" value="logout">logout</button></span></li>'); 
        for (var b=0, blog; blog = blogs[b]; b++) {
            var ttl = blog.getTitle().getText();
            if (ttl.length > 20) {
            	ttl = ttl.substring(0, 20) + '...';
            }
            var url = blog.getHtmlLink().getHref();
   
            chck = (url.toLowerCase().indexOf('oac') > -1 || ttl.toLowerCase().indexOf('annotation') > -1) ? 'checked="true"' : '';
            span = '<span style="float:right"><input type="radio" name="blog_radio" id="br_'+url+'" ' + chck +'></span></li>';
            html ='<li class="blog_select_li">&nbsp; ' + ttl + span + '</li>';            
            $('#create_body').append(html);
            topinfo['blogs']['br_'+url] = [blog, chck != '' ? true : false];
            var postsUri = blog.getEntryPostLink().getHref();
            gdataSvc.getBlogPostFeed(postsUri, handleBlogPostFeed, log);
        }
    }
    $("#create_body li:even").addClass("alt");
}


function handleBlogPostFeed(resp) {
    var feeduri = resp.feed.getHtmlLink().getHref();
    topinfo['blogs']['br_'+feeduri].push(resp.feed);
    if (topinfo['blogs']['br_'+feeduri][1]) {
    	entries = resp.feed.getEntries();
    	for (e in entries) {
    		entry = entries[e];
    		text = entry.content.getText();
    		// process RDFA from text
    		load_commentAnno(text);
    	}
    	// XXX Now process continuation links

    }
    
}

function postToBlog(title, rdfa, typ) {
    var newEntry = new google.gdata.blogger.BlogPostEntry({
        title: { type: 'text', text: title},
        content: { type: 'text', text: rdfa},
        categories: [ {scheme: 'http://www.blogger.com/atom/ns#', term: typ} ]
    });

    var which = $('#create_body input[name="blog_radio"]:radio:checked').attr('id');
    var bFO = topinfo['blogs'][which][2];
    bFO.insertEntry(newEntry,
    		function() {
    			// clean up
    			alert('Posted successfully.');
    			// and push new anno to list
    			
    			var postsUri = bFO.getEntryPostLink().getHref();
    			
    			gdataSvc.getBlogPostFeed(postsUri, handleBlogPostFeed, log);
    			
    	},log);
}



