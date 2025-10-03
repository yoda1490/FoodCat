fullUrl = ""

function initApp() {
  loadStatus();
  setInterval(loadStatus, 10000);
  loadSettings();
}


function loadStatus(){
  $('.spinnerGlobal').show();
  $.getJSON( fullUrl+"/status", function( data ) {
    $.each( data, function( key, val ) {
      $('.status.'+key).html(val);
    });
    $('.spinnerGlobal').hide();
  });
  
}

function loadSettings(){
  $('.spinnerSettings .spinner').show();
  $.getJSON( fullUrl+"/settings", function( data ) {
    fillSettings(data);
    $('.spinnerSettings .spinner').hide();
  });
}

function fillSettings(data){
  if(typeof(data.timezone) != 'undefined'){
    $('input[name="timezone"]').val(data.timezone);
  }
  if(typeof(data.ntpServer) != 'undefined'){
    $('input[name="ntpServer"]').val(data.ntpServer);
  }
  if(typeof(data.feedTime) != 'undefined'){
    $('input[name="feedTime"]').val(data.feedTime/1000);
  }
  if(typeof(data.ledStatus) != 'undefined'){
    $('input[name="ledStatus"]').prop("indeterminate", false);
    $('input[name="ledStatus"]').prop( "checked", data.ledStatus);
  }
  if(typeof(data.login) != 'undefined'){
    $('input[name="login"]').val(data.login);
  }
  if(typeof(data.schedules) != 'undefined'){
    $('form#schedules').empty();
    $.each(data.schedules, function(i,j){
      html='<label>Schedule '+i+': <select name="slot_'+i+'" onchange="setSchedule('+i+', $(this).val())">';
      html+='<option value="----">Disabled</option>';
      for(h=0; h<24; h++){
        if(h<9)h="0"+h;
        html+='<option value="'+h+'00" >'+h+'h00</option>';
        html+='<option value="'+h+'30">'+h+'h30</option>';
      }
      html+='</select></label><br/>';
      $('form#schedules').append(html);
      $('form#schedules select[name="slot_'+i+'"] option[value="'+j+'"]').prop('selected', true);
    });
  }
}

function buttonPress(target) {
  $('.spinnerActions .spinner').show();
  $.ajax({
    type: "GET",
    url: fullUrl+"/"+target,
    async: true,
    success: function (text) {
      if (text == 0) {
        if(target == "feed"){
          alert('Motor stuck !');
        }else{
          alert('An error occured in action !');
        }
      } else if (text == 1) {
        console.log(target + ': OK');
      } else {
        alert('Unknown error !');
      }
      $('.spinnerActions .spinner').hide();
    },
    fail: function (text) {
      alert('Error: ' + xhr.status);
      $('.spinnerActions .spinner').hide();
    }
  });
}

function setLed(status) {
  $('.spinnerGlobal').show();
  $.getJSON( fullUrl+"/settings/led/"+Number(status), function( data ) {
    fillSettings(data);
    $('.spinnerGlobal').hide();
  });
}

function setFeedTime(duration) {
  $('.spinnerGlobal').show();
  $.getJSON( fullUrl+"/settings/feedTime/"+(duration*1000), function( data ) {
    fillSettings(data);
    $('.spinnerGlobal').hide();
  });
}

function setSchedule(schedule, value) {
  $('.spinnerGlobal').show();
  $.getJSON( fullUrl+"/schedules/"+schedule+"/"+value, function( data ) {
    fillSettings(data);
    $('.spinnerGlobal').hide();
  });
}

function setAuth() {
  $('.spinnerGlobal').show();
  $.getJSON( fullUrl+"/settings/auth/"+$('input[name="login"]').val()+":"+$('input[name="password"]').val(), function( data ) {
    fillSettings(data);
    location.reload();
    $('.spinnerGlobal').hide();
  });
}

function reboot() {
  $('.spinnerGlobal').show();
  $.getJSON( fullUrl+"/reboot", function( data ) {
    location.reload();
    $('.spinnerGlobal').hide();
  });
}

function setNtp() {
  $('.spinnerGlobal').show();
  $.getJSON( fullUrl+"/settings\/ntp/timezone/"+$('input[name="timezone"]').val(), function( data ) {
    fillSettings(data);
    
  });
  $.getJSON( fullUrl+"/settings\/ntp/server/"+$('input[name="ntpServer"]').val(), function( data ) {
    fillSettings(data);
    $('.spinnerGlobal').hide();
  });
}



function saveSchedule(schedule) {
  alert("Not implemented yet");
}