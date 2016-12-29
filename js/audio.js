function stopAudio()
{
	$('#audio-player').remove();
	$('.button-play').removeClass('btn-primary').removeClass('playing');
	$('.button-play .glyphicon').removeClass('glyphicon-stop').addClass('glyphicon-play');
	$('.button-play .play-label').text('Play sample');
}

function playAudio(audio)
{
	$('body').append('<audio id="audio-player"><source src="'+audio+'" /></audio>');
	$('#audio-player')[0].onended = function()
	{
		stopAudio();
	}
	$('#audio-player')[0].play();
}

$(function(){
	$('.button-play').click(function(e){
		var button = $(e.target);
		button.blur();
		if (button.hasClass('playing'))
			stopAudio();
		else
		{
			stopAudio();
			button.addClass('btn-primary').addClass('playing');
			button.children('.glyphicon').removeClass('glyphicon-play').addClass('glyphicon-stop');
			button.children('.play-label').text('Stop');
			playAudio(button.data('audio'));
		}
	});
});

