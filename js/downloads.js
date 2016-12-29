/*
Fetch GitHub releases by Tero Lindeman (kometbomb), MIT License applies
*/

var my_repo = 'kometbomb/klystrack';

$(function(){
	if (navigator.oscpu)
	{
		if (navigator.oscpu.match(new RegExp('^Windows', 'i')))
			$('.dl-default').addClass('dl-arch-win32 dl-ext-exe');
		else if (navigator.oscpu.match(new RegExp('^Linux.*x86_64', 'i')))
			$('.dl-default').addClass('dl-arch-amd64 dl-ext-deb');
		else if (navigator.oscpu.match(new RegExp('^Linux.*i[3-6]86', 'i')))
			$('.dl-default').addClass('dl-arch-i386 dl-ext-deb');
	}
	
	$.ajax({
		url:'https://api.github.com/repos/'+my_repo+'/releases', 
		method:'GET',
		success:function(data){
			for (var i in data)
			{
				var release = data[i];
				
				// Skip pre-releases
				//if (release.prerelease)
				//	continue;
			
				$('.download-list a.dl-info').attr('href', release.html_url);
				$('.dl-latest-release-name').text(release.name);
				
				var clean_date = release.published_at.match(/^([^T]+)/);
				
				$('.dl-latest-release-time').text(clean_date[1]);
				
				$('.download-list a').each(function(idx, el){
					var link = $(el);
					
					var classes = el.className.split(' ');
					
					var ext = null;
					var arch = null;
					
					for (var c in classes)
					{
						var m;
						var cl = classes[c];
						if (m = cl.match(/^dl-ext-(.*)$/))
							ext = m[1];
						else if (m = cl.match(/^dl-arch-(.*)$/))
							arch = m[1];
					}
					
					if (!ext || !arch)
						return;
					
					if (ext == 'exe' && arch == 'win32')
						arch = '';
					
					link.attr('href', 'https://github.com/'+my_repo+'/releases/');
					
					for (var f in release.assets)
					{
						var asset = release.assets[f];
						var url = asset.browser_download_url;
						
						if (url.match(new RegExp(arch+'.*'+ext+'$', 'i')))
						{
							link.attr('href', url);
							link.children('.badge').each(function(idx,badge){ $(badge).text(asset.download_count+' downloads'); });
							break;
						}
					}
				});
				
				break;
			}
		},
		error:function()
		{
			$('.download-list a').attr('href', 'https://github.com/'+my_repo+'/releases/');
		}
	});
});
