var maxcolumns = 16;
var version = "CAP"
var interval_id = 0;
var flashing = 0;

var ws = null;
var z;
var c ;            
var graph;
var xPadding = 0;
var yPadding = 0;
var alive = 0;
var count = 0;

var FRAMETYPE = {
		FRC_SET : 0x01,
		FRC_UPDATE : 0x02,
		FRC_GET : 0x03
};

var FRAMECOMMAND = {
		FRC_NONE :0x00,
		FRC_ANIMATION_SPEED	:	0x01	,
		FRC_FADE_SPEED	:	0x02	,
		FRC_FADE_ENABLE	:	0x03	,
		FRC_FLICKER_ENABLE	:	0x04	,
		FRC_FLICKER_SPEED	:	0x05	,
		FRC_TEXT_SPEED	:	0x06	,
		FRC_GIF_VALUE	:	0x07	,
		FRC_LCD_ENABLE	:	0x08	,
		FRC_FLASHLIGHT_ENABLE	:	0x09	,
		FRC_CONSOLE_ENABLE	:	0x0a	,
		FRC_SLIDESHOW_ENABLE	:	0x0b	,
		FRC_BRIGHTNESS_VALUE	:	0x0c	,
		FRC_SLIDESHOW_SPEED	:	0x0d	,
		FRC_SYNC_ALL	:	0x0e	,
		FRC_SLIDESHOW_CHOISE	:	0x0f	,
		FRC_TEXT_BIG	:	0x10	,
		FRC_TEXT_SCROLL_MULTIPLE	:	0x11	,
		FRC_AP_USER	:	0x12	,
		FRC_AP_PASSWORD	:	0x13	,
		FRC_CONNECT_CLIENT_SAVE	:	0x14	,
		FRC_TEXT_COLOR	:	0x15	,
		FRC_COLUMNS	:	0x16	,
		FRC_BUILD_NR	:	0x17	,
		FRC_BUILD_TYPE	:	0x18	,
		FRC_CLIENT_USER	:	0x19	,
		FRC_CLIENT_PASSWORD	:	0x1a	,
		FRC_TEXT_WIDE_DATA	:	0x1b	,
		FRC_TEXT_DATA	:	0x1c	,
		FRC_ANIMATION_OFFSET	:	0x1d	,
		FRC_ANIMATION_COUNTER	:	0x1e	,
		FRC_FRAME_COUNTER	:	0x1f	,
		FRC_ANIMATION_FRAMECOUNT	:	0x20	,
		FRC_TEXT_OFFSET	:	0x21	,
		FRC_BATTERY_VALUE	:	0x22	,
		FRC_IP_VALUE	:	0x23	,
		FRC_CONNECT_CLIENT_NOW : 0x24, 
		FRC_OPEN_DATA : 0x25,
		FRC_INVALID:0x26
};


navigator.sayswho = (function(){
var ua= navigator.userAgent, tem, 
M= ua.match(/(opera|chrome|safari|firefox|msie|trident(?=\/))\/?\s*(\d+)/i) || [];
if(/trident/i.test(M[1])){
    tem=  /\brv[ :]+(\d+)/g.exec(ua) || [];
    return 'IE '+(tem[1] || '');
}
if(M[1]=== 'Chrome'){
    tem= ua.match(/\bOPR\/(\d+)/)
    if(tem!= null) return 'Opera '+tem[1];
}
M= M[2]? [M[1], M[2]]: [navigator.appName, navigator.appVersion, '-?'];
if((tem= ua.match(/version\/(\d+)/i))!= null) M.splice(1, 1, tem[1]);
return M[0];
})();



function toggleFullScreen() {
  if (!document.fullscreenElement &&    // alternative standard method
      !document.mozFullScreenElement && !document.webkitFullscreenElement && !document.msFullscreenElement ) {  // current working methods
    if (document.documentElement.requestFullscreen) {
      document.documentElement.requestFullscreen();
    } else if (document.documentElement.msRequestFullscreen) {
      document.documentElement.msRequestFullscreen();
    } else if (document.documentElement.mozRequestFullScreen) {
      document.documentElement.mozRequestFullScreen();
    } else if (document.documentElement.webkitRequestFullscreen) {
      document.documentElement.webkitRequestFullscreen(Element.ALLOW_KEYBOARD_INPUT);
    }
  } 
}


function startMyInterval() {
	clearInterval(interval_id);
    interval_id = setInterval(function(){
    	
        if(count > 5) {
        	ws.send("PING");
            count = 0;
        }
        
        if(alive > 10 && ws.readyState == 1 && flashing == 0) {
             ws.close();
             ws = null;
             WebSocketTest();
             alive = 0;
        }
        
        if(alive > 15  && flashing == 1) {
        	alert("Flashing done");
        	alive = 0;
        	location.reload();
        }
        
        count++;
        alive++;
    }, 1000);

}

function set_flashing(data) {
	flashing = data;
}

function get_columns() {
	return maxcolumns;
} 

function analyse_leddata(bufView) {
  	for(var i = 4; i < bufView.length; i+=3) {
    	//console.log(bufView[i+1] + "-" + bufView[i] + "-" + bufView[i+2]);
        var temp0 = 0;
        var temp1 = 0;
        var temp2 = 0;
        
    
    	if(bufView[i] != 0)
    	temp0 = bufView[i]+ 100;
    	if(temp0 > 255)
    	  temp0 = 255;

		if(bufView[i+1] != 0)
    	temp1 = bufView[i+1] + 150;
    	if(temp1 > 255)
    	  temp1 = 255;

		if(bufView[i+2] != 0)
    	temp2 = bufView[i+2] + 150;
    	if(temp2 > 255)
    	  temp2 = 255;
    	        	
    	$("#led"+ count).css("background-color", "rgb(" + temp0 + "," + temp1 + "," + temp2 + ")" ); 	
    	count++;        	
    }
}



function analyze_blockdata(size, arr, offset, totalsize) {
	if(size < 2 && offset + size <= totalsize) {
		console.log("Error, could process data"); 
		return;
	}
	
	console.log("Data type:" + arr[offset+2]);
	switch(arr[offset+2]) {
	    case FRAMECOMMAND.FRC_ANIMATION_SPEED:
        	receiveAnimationSpeed(arr[offset+3]);
	        break;
	    case FRAMECOMMAND.FRC_FADE_SPEED:
        	receiveFadeSpeed(arr[offset+3]);
	        break;   
	    case FRAMECOMMAND.FRC_FADE_ENABLE:
        	receiveFade(arr[offset+3]);
	        break;  
	    case FRAMECOMMAND.FRC_FLICKER_SPEED:
        	receiveFlickerSpeed(arr[offset+3]);
	        break;  
	    case FRAMECOMMAND.FRC_FLICKER_ENABLE:
        	receiveFlicker(arr[offset+3]);
	        break;
	    case FRAMECOMMAND.FRC_TEXT_SPEED:
        	receiveTextSpeed(arr[offset+3]);
	        break;     
	    case FRAMECOMMAND.FRC_BRIGHTNESS_VALUE:
        	receiveBrightness(arr[offset+3]);
	        break;
	    case FRAMECOMMAND.FRC_GIF_VALUE:
        	receiveAnimation(arr[offset+3]);
	    break;
	    case FRAMECOMMAND.FRC_FLASHLIGHT_ENABLE:
        	console.log("Setting flashlight value:" + arr[offset+3]);
	    break;
	    case FRAMECOMMAND.FRC_CONSOLE_ENABLE:
        	receiveDebugConsole(arr[offset+3]);
	    break;
	    case FRAMECOMMAND.FRC_SLIDESHOW_ENABLE:
        	receiveRandom(arr[offset+3]);
	    break; 	        
	    case FRAMECOMMAND.FRC_BATTERY_VALUE:
    		var dat = String.fromCharCode.apply(null, arr);
			dat = dat.substring(offset+3, (size+offset)+1);
        	var myarr2 = dat.split(/=(.+)?/);
        	if(typeof myarr2[1] != 'undefined') {
        		console.log("Client:" + myarr2[0] + " val:" + myarr2[1]);
        		if(myarr2[0] != 0)
        		receiveBatteryClient(myarr2[0], myarr2[1]);
        	} else {
        		
        		var adcdat = arr[offset+4];
        		adcdat = adcdat + (arr[offset+3] << 8);
        		console.log("Got battery val" + adcdat);
            	receiveBattery(adcdat);
            }
	    break; 	
	    case FRAMECOMMAND.FRC_SLIDESHOW_SPEED:
        	receiveRandomSpeed(arr[offset+3]);
	    break; 	
	    case FRAMECOMMAND.FRC_IP_VALUE:
	    	var dat = String.fromCharCode.apply(null, arr);
			dat = dat.substring(offset+3, (size+offset)+1);
	    	if(dat == "0.0.0.0") {
        		$("#iptext").text("Status: Disconnected");
        	} else {
        		$("#iptext").text("Status: Connected - IP:" + dat);
        	}
	    break;
	    case FRAMECOMMAND.FRC_SYNC_ALL:
        	receiveSync(arr[offset+3]);
	    break; 	
	    case FRAMECOMMAND.FRC_TEXT_BIG:
        	receiveTextsize( arr[offset+3]);
	    break; 	
	    case FRAMECOMMAND.FRC_TEXT_SCROLL_MULTIPLE:
        	receiveTextspan(arr[offset+3]);
	    break; 	   
	    case FRAMECOMMAND.FRC_CONNECT_CLIENT_SAVE:
        	console.log("Setting connect client value:" + arr[offset+3]);
        	$("#clientconnect").val(arr[offset+3] == "1" ? "on" : "off").trigger('create').slider("refresh");
	    break;
	
		case FRAMECOMMAND.FRC_AP_USER:
			var dat = String.fromCharCode.apply(null, arr);
			dat = dat.substring(offset+3, (size+offset)+1);
			console.log("AP USER-" + dat+"-");	
        	receiveAPuser(dat);
	    break;
	    case FRAMECOMMAND.FRC_AP_PASSWORD:
			var dat = String.fromCharCode.apply(null, arr);
			dat = dat.substring(offset+3, (size+offset)+1);
			console.log("AP PASS-" + dat+"-");	
        	receiveAPpassword(dat);
	    break;
	    case FRAMECOMMAND.FRC_CLIENT_USER:
			var dat = String.fromCharCode.apply(null, arr);
			dat = dat.substring(offset+3, (size+offset)+1);
			console.log("CLIENT USER" + dat);	
        	receiveCLuser(dat);
	    break;
	    case FRAMECOMMAND.FRC_CLIENT_PASSWORD:
			var dat = String.fromCharCode.apply(null, arr);
			dat = dat.substring(offset+3, (size+offset)+1);
			console.log("CLIENT PASS" + dat);	
        	receiveCLpassword(dat);
	    break;
	    case FRAMECOMMAND.FRC_COLUMNS:
			maxcolumns = arr[offset+3];
			console.log("Columns: " + maxcolumns);	
	    break;
	    case FRAMECOMMAND.FRC_BUILD_NR:
		    var adcdat = arr[offset+4];
	         adcdat = adcdat + (arr[offset+3] << 8);
	         console.log("Buildnr" + arr[offset+4]);
	         receiveBuildnr(adcdat);
	    break;
	    case FRAMECOMMAND.FRC_BUILD_TYPE:
			var dat = String.fromCharCode.apply(null, arr);
			var version = dat.substring(offset+3, (size+offset)+1);
			if(version == "USB") {
        		$("#title").html("LED Lines - LED Usb adapter!");
        		$("#console_container").hide();
        		$("#debug_console_container").hide();
        	}
        	
        	if(version == "CAP") {
        		$("#title").html("LED Lines - LED Cap!");
        		$("#console_container").hide();
        		$("#debug_console_container").hide();
        	}
        	
        	if(version == "POCKET") {
        		$("#title").html("LED Lines - Pocket Edition!");
        		$("#console_container").hide();
        		$("#debug_console_container").hide();
        	}
        	
        	if(version == "SHIRT") {
        		$("#title").html("LED Lines - LED Shirt!");
        	}
        	
        	if(version == "DEV") {
        		$("#title").html("LED Lines - Development board!");
        	}
	    break; 
	
	    default:
	        console.log("Default" + arr[offset+2]);
	}
	
	if(totalsize > offset + size + 1) {
		if(typeof arr[offset+size+1] != 'undefined') {
			//console.log("More data available newsize:" + arr[offset+size+1]);
			analyze_blockdata(arr[offset+size+1], arr, offset+size+1, totalsize);
		}
	}
	
}

function analyze_data(blob)
{
    var myReader = new FileReader();
    myReader.readAsArrayBuffer(blob)
    
    myReader.addEventListener("loadend", function(e)
    {
        var buffer = e.srcElement.result;//arraybuffer object
        var bufView = new Uint8Array(buffer);
        var count = 0;
        if(bufView[0] == 'L' && bufView[1] == 'E' && bufView[2] == 'D') {
        	console.log("LED Data");
        	analyse_leddata(bufView);
        }  else {
        	//console.log("Data available newsize:" + bufView[0]);
        	analyze_blockdata(bufView[0], bufView, 0, bufView.length);
        	
        }
       
    });
}
	
	

 function WebSocketTest()
 {
    if ("WebSocket" in window)
    {
       if(ws == null) {
            ws = new WebSocket("ws://" + location.host + "/echo");
            toast("Connecting...", true);
       } else {
            ws.onmessage = null;
            ws.onopen = null;
            ws.onerror = null;
            ws.onclose = null;
       }
       
           ws.onopen = function()
           {
               toast(null, false);
                sendDataU8(FRAMECOMMAND.FRC_OPEN_DATA, 1);
                
                $.get("/gifoptions", function(data) {
                	//alert("got data" + data);
                	var dat= data.substring(data.indexOf("{"));
                	var arr = eval(data);
                    var i = 0;
                    //$("#animation").empty();
                    $("#animation_picker").empty();
                    $("#random_choise").empty();
                	$.each(arr, function(key, value) {
                		//alert(arr[0]);
                		if(i == 0) {
                			var append = "";
                			if(value.indexOf("-S") > -1) {
                				append = " selected ";
                				value=value.substring(0, value.length -2);
                			}
           			        $("#random_choise").append('<option ' + append + ' value="0">All</option>');
                		}
                		
                		if(i > 0) {
                			var append = "";
                			if(value.indexOf("-S") > -1) {
                				append = " selected ";
                				value=value.substring(0, value.length -2);
                			}
                			$("#animation_picker").append("<span style='float:left;'><img class='animation_pick'  data-val='" + (i+1) + "' style='margin-left:1px; border: 3px solid transparent;' onclick='javascript:sendAnimationVal(" + (i) + ");' height=30 src='/gif/" + value + "' /></span>");
                			value=value.substring(1);
                			 var optTempl = '<option ' + append + ' value="' + i + '">'+value+'</option>';            
        					$("#random_choise").append(optTempl)
                			
                		}
                		value=value.replace(".gif", "");
                		
					    //$("#animation").append('<option value=' + (i+1) + '>' + value + '</option>');
					    i++;
					});
					 $("#random_choise").selectmenu();
    				 $("#random_choise").selectmenu('refresh', true);
               	});
           };
		
           ws.onmessage = function (evt) 
           { 
             	console.log("Received data");
           	   
           		if(!flashing)
           	   	toast(null, false);
           	   
               alive = 0
               flash_message_callback(evt);
               
              var received_msg = evt.data;
              if(!(typeof evt.data === "string")) {
              	analyze_data(evt.data);
              	return;
              } 
              
              var myarr = received_msg.split(/:(.+)?/);
              switch(myarr[0]) {
                case "LED":
                    receiveLed(myarr[1]);
                    break;
                default:
                 break;                                  
              }
              //console.log(received_msg);
           };
		
	       ws.onerror = function(event)
	       {
	       		console.log("ERROR");
	            ws.close();
	            ws = null;
    			WebSocketTest();
	       }
           ws.onclose = function(event)
           { 
    			//alert(event.code);
    			console.log("CLOSE");
    			//ws.close();
    			//ws = null;
    			//ws = new WebSocket("ws://84.31.4.171:9000/echo");
    			//toast("Connecting...", true);
    			//WebSocketTest();
           };
       
    }
    else
    {
       alert("WebSocket NOT supported by your Browser!");
    }
 }
 
 var dat = null;
 var dat2=null;
 
var toast=function(msg, stay){
 	if(dat != null && msg != null && stay == 1) {
    	return;
    }
    
    if(dat != null) {
       dat.remove();
       dat = null;
    }

  if(dat2 != null) {
   		dat2.remove();
   		dat2 = null;
    }
    
   
    
    if(msg == null) {
        return;
    }
    
dat = $("<div class='ui-loader ui-overlay-shadow ui-body-e ui-corner-all'></div>")
.css({ display: "block", 
	opacity: 0.90, 
	position: "fixed",
	background: "black",
	"text-align": "center",
	width: "100%",
	height: "100%",
	left: 0,
	top: 0})
    .hide().appendTo( $.mobile.pageContainer ).fadeIn(400).delay( 1500 );
    if(!stay) {
        dat.fadeOut(400, function(){
	        $(this).remove();
	        dat = null;
        });
    }




dat2 = $("<div class='ui-loader ui-overlay-shadow ui-body-e ui-corner-all'><h3>"+msg+"</h3></div>")
.css({ display: "block", 
	opacity: 0.90, 
	position: "fixed",
	padding: "7px",
	background: "white",
	"text-align": "center",
	width: "270px",
	left: ($(window).width() - 284)/2,
	top: $(window).height()/2 })

.hide().appendTo( $.mobile.pageContainer ).fadeIn(400).delay( 1500 );
	    if(!stay) {
        dat2.fadeOut(400, function(){
	        $(this).remove();
	        dat = null;
         });
        }
}

function sendDataU8(type, data) {
	var bufView = new Uint8Array(4);
	bufView[0] = 3;
	bufView[1] = FRAMETYPE.FRC_SET
	bufView[2] = type;
	bufView[3] = data;
	ws.send(bufView);
}
	
  
function sendDataString(type, data) {
	var bufView = new Uint8Array(3 + data.length);
	bufView[0] = 3 + data.length;
	bufView[1] = FRAMETYPE.FRC_SET
	bufView[2] = type;
	for(var i = 0; i < data.length; i++) {
		bufView[i+3] = data.charCodeAt(i);
	}
	ws.send(bufView);
}

    
var curbright = 0;

function receiveBrightness(data) {
	curbright = data;
	//console.log("BRI" + data);
	if(parseInt(data) <= parseInt($("#brightness").attr("max"))) {
    	$("#brightness").val(data).trigger('create').slider("refresh");
    }
}

function sendAnimationVal(val) {
    sendDataU8(FRAMECOMMAND.FRC_GIF_VALUE, parseInt(val));
    tetris_stop();
   	snake_stop();
   	var data = "0";
   	$("#game_snake").val(data == "1" ? "on" : "off").trigger('create').slider("refresh");
    $("#game_tetris").val(data == "1" ? "on" : "off").trigger('create').slider("refresh");
}

function receiveAnimation(data) {
	var val = ((parseInt(data)));
	if(val == 0) {
		$("#text_speed_container").show();
		$("#animation_speed_container").hide();
		$("#random_toggle").hide();
		$("#random_speed_container").hide();
		$("#random_choise_container").hide();
		$("#text_color_container").show();
		$("#text_scroll_container").show();
		$("#text_font_container").show();
		
	} else {
		$("#text_speed_container").hide();
		$("#animation_speed_container").show();
		$("#random_toggle").show();
		$("#random_choise_container").show();
		$("#text_color_container").hide();
		$("#text_scroll_container").hide();
		$("#text_font_container").hide();
	}
	var sel = (parseInt(data) + 1);
	$(".animation_pick").each(function(key,value) {
		var i = $(this).data('val');
		if(i == sel) {
			$(this).css('border', '3px dotted #0B85A1');
		} else {
			$(this).css('border', '3px solid transparent');
		}
	});
    //$("#animation").val(((parseInt(data)) + 1)).selectmenu("refresh", true);
}
  
function receiveAnimationSpeed(data) {
    //console.log("SS" + (parseInt($("#animation_speed").attr("max")) - data + 1));
    $("#animation_speed").val((parseInt($("#animation_speed").attr("max")) - data + 1)).trigger('create').slider("refresh");
}
function clearScreen() {
		sendText("       ");
}
function receiveFlicker(data) {
    //console.log(data);
    if(data == "1") {
    	$("#flicker_speed_container").show();
    } else {
		$("#flicker_speed_container").hide();
	}
    $("#flicker").val(data == "1" ? "on" : "off").trigger('create').slider("refresh");
}
    
function receiveLed(data) {
    //console.log("data:" + data);
    //console.log("data[0]" + data[0]);
}


function receiveBatteryClient(user, data) {
	console.log("Battery volt:" + data);
	if(data < 701) {
	data = 701;
	}
	if(data > 700) {
		var perc = 100;
		if(version == "USB") {
			perc = data - 940; //700;
			perc = perc * 1.120; //3.24;
		} else {
			perc = data - 700;
			perc = perc / 3.24;
		}
		console.log("Percent" + perc);
		
		if($("#battery" + user).length) {
			$("#battery" + user + " .batteryperc").width(perc + "%");
		} else {
			console.log("Cloning new div");
		    $("#battery").clone().attr('id', 'battery' + user).insertAfter("#battery");  
		    $("#battery" + user + " .batteryperc").width(perc + "%");
		}
		
	}
}

function receiveBattery(data) {
	//console.log("Battery volt:" + data);
	if(data > 700) {
		var perc = 100;
		if(version == "USB") {
			perc = data - 940; //700;
			perc = perc * 1.120; //3.24;
		} else {
			perc = data - 700;
			perc = perc / 3.24;
		}
		console.log("Percent" + perc);
		$("#battery .batteryperc").width(perc + "%");
	}
	
}

function receiveBuildnr(data) {
	console.log("Buildnr" + data);
 	$("#buildnr").html("(" + data + ")");
}
    
function receiveFade(data) {
    //console.log(data);
    if(data == "1") {
    	$("#fade_speed_container").show();
    } else {
		$("#fade_speed_container").hide();
	}
    $("#fade").val(data == "1" ? "on" : "off").trigger('create').slider("refresh");

}
function receiveFlickerSpeed(data) {
    //console.log(data + "WW");
    $("#flicker_speed").val(data).trigger('create').slider("refresh");
}
function receiveFadeSpeed(data) {
    //console.log(data + "WW");
    $("#fade_speed").val(data).trigger('create').slider("refresh");
}
function receiveTextSpeed(data) {
    //console.log("TSS" + (parseInt($("#text_speed").attr("max")) - data + 1));
    $("#text_speed").val((parseInt($("#text_speed").attr("max")) - data + 1)).trigger('create').slider("refresh");
    
}
function receiveTxtColor(data) {
	//console.log("Received txt color:" + data);
    $("#font-color").val("#" + data).trigger('create').slider("refresh");
}

function receiveAPuser(data) {
    $("#input_ap_ssid").val(data);
}
function receiveAPpassword(data) {
    $("#input_ap_password").val(data);
}
function receiveCLuser(data) {
    $("#input_client_ssid").val(data);
}
function receiveCLpassword(data) {
    $("#input_client_password").val(data);
} 

function receiveDebugConsole(data) {
    $("#console_output").val(data == "1" ? "on" : "off").trigger('create').slider("refresh");
}
    
function sendAPuser(data) {
    sendDataString(FRAMECOMMAND.FRC_AP_USER, data);
}
function receiveRandom(data) {
	if(data == 0x01) {
    	$("#random_speed_container").show();
    	$("#random_choise_container").show();
    } else {
		$("#random_speed_container").hide();
		$("#random_choise_container").hide();
	}
    $("#animation_random").val(data == 1 ? "on" : "off").trigger('create').slider("refresh");
}
function receiveSync(data) {
   $("#sync_all").val(data == "1" ? "on" : "off").trigger('create').slider("refresh");
}

function receiveTextsize(data) {
   $("#textsize").val(data == "1" ? "on" : "off").trigger('create').slider("refresh");
}
function receiveTextspan(data) {
   $("#textspanmultiple").val(data == "1" ? "on" : "off").trigger('create').slider("refresh");
}
   
    
function receiveRandomSpeed(data) {
    $("#random_speed").val((parseInt($("#random_speed").attr("max")) - data + 1)).trigger('create').slider("refresh");
}
function sendAPpassword(data) {
	sendDataString(FRAMECOMMAND.FRC_AP_PASSWORD, data);
}

function sendCLuser(data) {
	sendDataString(FRAMECOMMAND.FRC_CLIENT_USER, data);
    
}
function sendCLpassword(data) {
    sendDataString(FRAMECOMMAND.FRC_CLIENT_PASSWORD, data);
}

function receiveText(data) {
	if(data != "       ")
    $("#input_text").val(data);
}
    
    
function sendText(data) {
    //ws.send("TEXT:" + data + " ");
    sendDataString(FRAMECOMMAND.FRC_TEXT_DATA, data + " ");
    
    tetris_stop();
    snake_stop();
    
    var datn = data.replace(/ /g, "_");
    if($("#textshort #" + datn).html() == null || $("#textshort #" + datn).html() == "") {
    	$("#textshort").append("<span id='" + datn + "'>" + data + "</span>");
    	$("#textshort #" + datn).on("tap", function() {
	    	sendText($(this).html());
	    });
    }
    data = "0";
    $("#game_snake").val(data == "1" ? "on" : "off").trigger('create').slider("refresh");
    $("#game_tetris").val(data == "1" ? "on" : "off").trigger('create').slider("refresh");
}

function send_leddata(data) {
	var u32 = new Uint32Array(1, 0x80000);
    var sendstr = new ArrayBuffer(520); // 256-byte ArrayBuffer.
    var dv = new DataView(sendstr);
    dv.setUint8(0, 0x4C);
    dv.setUint8(1, 0x45);
    dv.setUint8(2, 0x44);
    dv.setUint8(3, 0x53);
    dv.setUint8(4, 0x3a);    
 	var count = 5;
 	var inputcount = 0;
 	
    for(var x = 0; x < 16;  x++) {
    	for(var y = 0; y < 8;  y++) {
    		for(var color = 0; color < 3; color++) {
        		dv.setUint8(count, data[inputcount]);
        		count++;
        		inputcount++;
        	}
        }  
    }      
    var bb = new Blob([new Uint8Array(sendstr)]);
    ws.binaryType = "blob";
    ws.send(bb);
    //console.log("Writing LEDS");
}

 $(document).on("scrollstart",function(){
  		toggleFullScreen();
});
         
window.requestAnimFrame = window.requestAnimationFrame || window.mozRequestAnimationFrame;     
                          
var lastupdate = new Date();

var isMobile = false; //initiate as false
// device detection
if(/(android|bb\d+|meego).+mobile|avantgo|bada\/|blackberry|blazer|compal|elaine|fennec|hiptop|iemobile|ip(hone|od)|ipad|iris|kindle|Android|Silk|lge |maemo|midp|mmp|netfront|opera m(ob|in)i|palm( os)?|phone|p(ixi|re)\/|plucker|pocket|psp|series(4|6)0|symbian|treo|up\.(browser|link)|vodafone|wap|windows (ce|phone)|xda|xiino/i.test(navigator.userAgent) 
    || /1207|6310|6590|3gso|4thp|50[1-6]i|770s|802s|a wa|abac|ac(er|oo|s\-)|ai(ko|rn)|al(av|ca|co)|amoi|an(ex|ny|yw)|aptu|ar(ch|go)|as(te|us)|attw|au(di|\-m|r |s )|avan|be(ck|ll|nq)|bi(lb|rd)|bl(ac|az)|br(e|v)w|bumb|bw\-(n|u)|c55\/|capi|ccwa|cdm\-|cell|chtm|cldc|cmd\-|co(mp|nd)|craw|da(it|ll|ng)|dbte|dc\-s|devi|dica|dmob|do(c|p)o|ds(12|\-d)|el(49|ai)|em(l2|ul)|er(ic|k0)|esl8|ez([4-7]0|os|wa|ze)|fetc|fly(\-|_)|g1 u|g560|gene|gf\-5|g\-mo|go(\.w|od)|gr(ad|un)|haie|hcit|hd\-(m|p|t)|hei\-|hi(pt|ta)|hp( i|ip)|hs\-c|ht(c(\-| |_|a|g|p|s|t)|tp)|hu(aw|tc)|i\-(20|go|ma)|i230|iac( |\-|\/)|ibro|idea|ig01|ikom|im1k|inno|ipaq|iris|ja(t|v)a|jbro|jemu|jigs|kddi|keji|kgt( |\/)|klon|kpt |kwc\-|kyo(c|k)|le(no|xi)|lg( g|\/(k|l|u)|50|54|\-[a-w])|libw|lynx|m1\-w|m3ga|m50\/|ma(te|ui|xo)|mc(01|21|ca)|m\-cr|me(rc|ri)|mi(o8|oa|ts)|mmef|mo(01|02|bi|de|do|t(\-| |o|v)|zz)|mt(50|p1|v )|mwbp|mywa|n10[0-2]|n20[2-3]|n30(0|2)|n50(0|2|5)|n7(0(0|1)|10)|ne((c|m)\-|on|tf|wf|wg|wt)|nok(6|i)|nzph|o2im|op(ti|wv)|oran|owg1|p800|pan(a|d|t)|pdxg|pg(13|\-([1-8]|c))|phil|pire|pl(ay|uc)|pn\-2|po(ck|rt|se)|prox|psio|pt\-g|qa\-a|qc(07|12|21|32|60|\-[2-7]|i\-)|qtek|r380|r600|raks|rim9|ro(ve|zo)|s55\/|sa(ge|ma|mm|ms|ny|va)|sc(01|h\-|oo|p\-)|sdk\/|se(c(\-|0|1)|47|mc|nd|ri)|sgh\-|shar|sie(\-|m)|sk\-0|sl(45|id)|sm(al|ar|b3|it|t5)|so(ft|ny)|sp(01|h\-|v\-|v )|sy(01|mb)|t2(18|50)|t6(00|10|18)|ta(gt|lk)|tcl\-|tdg\-|tel(i|m)|tim\-|t\-mo|to(pl|sh)|ts(70|m\-|m3|m5)|tx\-9|up(\.b|g1|si)|utst|v400|v750|veri|vi(rg|te)|vk(40|5[0-3]|\-v)|vm40|voda|vulc|vx(52|53|60|61|70|80|81|83|85|98)|w3c(\-| )|webc|whit|wi(g |nc|nw)|wmlb|wonu|x700|yas\-|your|zeto|zte\-/i.test(navigator.userAgent.substr(0,4))) isMobile = true;
    
    

(function loop() {
      var now = new Date();
      if ( now.getTime() - lastupdate.getTime() > 5000 ) {
          // browser was suspended and did come back to focus
          	if(isMobile) {
				toast("Connecting...", true);
				ws.close();
	             ws = null;
	             WebSocketTest();
	             
			    startMyInterval();
			}
      }
      
       if ( now.getTime() - lastupdate.getTime() > 60000 ) {
          // browser was suspended and did come back to focus
          	toast("Connecting...", true);
			location.reload(false);

      }
      
      lastupdate = now;

      window.requestAnimFrame(loop);
})();
                          
                          
$(window).focus(function() {
	if(!isMobile)
		return;
		
	console.log("focus");
	clearInterval(interval_id);
    interval_id = 0;
    
    if (!interval_id) {
    	toast("Connecting...", true);
        startMyInterval();
    }
});

$(window).blur(function() {
	if(!isMobile)
		return;
		
	console.log("out focus");
    clearInterval(interval_id);
    interval_id = 0;
});
     
       
 $(document).on("pagecreate","#page1",function(){
    $.mobile.changePage.defaults.changeHash = false;
    $.mobile.hashListeningEnabled = false;
    $.mobile.pushStateEnabled = false;
    var browser = navigator.sayswho.toLowerCase();
    $('body').addClass(browser);
	
    $("#brightness").change(function() {
       if(curbright != $("#brightness").val()) {
    		sendDataU8(FRAMECOMMAND.FRC_BRIGHTNESS_VALUE, parseInt($("#brightness").val())); 	
        }
    });
	
    startMyInterval();
    
    $("#animation_speed").change(function() {
        var val = (parseInt($("#animation_speed").attr("max")) - parseInt($("#animation_speed").val()) + 1);
    	sendDataU8(FRAMECOMMAND.FRC_ANIMATION_SPEED, parseInt(val));
    });
    
    
    $("#random_speed").change(function() {
        var data = (parseInt($("#random_speed").attr("max")) - parseInt($("#random_speed").val()) + 1);
        sendDataU8(FRAMECOMMAND.FRC_SLIDESHOW_SPEED, parseInt(data));
    });
    
    $("#text_speed").change(function() {
        var data = (parseInt($(this).attr("max")) + 1 - parseInt($(this).val()));
        sendDataU8(FRAMECOMMAND.FRC_TEXT_SPEED, parseInt(data));
    });
    
    $("#button_send").on("tap", function() {
        sendText($("#input_text").val());
    });
    $("#button_ap_save").on("tap", function() {
        sendAPuser($("#input_ap_ssid").val());
        sendAPpassword($("#input_ap_password").val());
    });
    
    $("#button_wifi_connect").on("tap", function() {
       sendDataU8(FRAMECOMMAND.FRC_CONNECT_CLIENT_NOW, 1);
    });
    
    $("#button_client_save").on("tap", function() {
        sendCLuser($("#input_client_ssid").val());
        sendCLpassword($("#input_client_password").val());
    });
    
    $("#flicker_speed").change(function() {
        sendDataU8(FRAMECOMMAND.FRC_FLICKER_SPEED, parseInt($(this).val()));
    });
    
    $("#fade_speed").change(function() {
        //console.log("change");
        sendDataU8(FRAMECOMMAND.FRC_FADE_SPEED, parseInt($(this).val()));
    });
    
    $("#flicker").change(function() {
        var val = ($(this).val() == "on" ? 1 : 0);
        sendDataU8(FRAMECOMMAND.FRC_FLICKER_ENABLE, parseInt(val));
    });
    
    
    $("#textspanmultiple").change(function() {
        var val = ($(this).val() == "on" ? 1 : 0);
        sendDataU8(FRAMECOMMAND.FRC_TEXT_SCROLL_MULTIPLE, parseInt(val));
    });
        
    $("#extra_bright").change(function() {
    	if($(this).val() == "on") {
        $("#brightness").attr("max", "250");
        } else {
        $("#brightness").attr("max", "100");
        }
    });
    
    
    $("#textsize").change(function() {
        var val = ($(this).val() == "on" ? 1 : 0);
        sendDataU8(FRAMECOMMAND.FRC_TEXT_BIG, parseInt(val));
    });
    
    $("#random_speed").change(function() {
    	$("#random_speed").val((parseInt($("#random_speed").attr("max")) - data + 1)).trigger('create').slider("refresh");
    });
    
    $("#font-color").change(function() {
        console.log("Color " + $(this).val());
        sendDataString(FRAMECOMMAND.FRC_TEXT_COLOR, $(this).val(), 6); 
    });
    
    $("#animation_random").change(function() {
        var val = ($(this).val() == "on" ? 1 : 0);
        sendDataU8(FRAMECOMMAND.FRC_SLIDESHOW_ENABLE, parseInt(val));
        
    });
    
    $("#sync_all").change(function() {
        var val = ($(this).val() == "on" ? 1 : 0);
        sendDataU8(FRAMECOMMAND.FRC_SYNC_ALL, parseInt(val));
    });
    
    
   
    $("#game_tetris").change(function() {
        
        if($(this).val() == "on") {
        	tetris_init();
        	snake_stop();
        } else {
        	tetris_stop();
        	snake_stop();
        }	

        var data = "0";
        $("#game_snake").val(data == "1" ? "on" : "off").trigger('create').slider("refresh");
        
    }); 
    
    $("#game_snake").change(function() {        
        if($(this).val() == "on") {
        	tetris_stop();
        	snake_init();
        } else {
        	tetris_stop();
        	snake_stop();
        }	

        var data = "0";
        $("#game_tetris").val(data == "1" ? "on" : "off").trigger('create').slider("refresh");
        
    });
  
    $("#game_up").on("tap", function() {
    	
	  	snake_setdirection(3);
	  	tetris_setdirection(3);
	  	if ("vibrate" in navigator) {
	  	navigator.vibrate(50);
	  	}
    });
    
    $("#game_right").on("tap", function() {
	  	
	  	snake_setdirection(0);
	  	tetris_setdirection(0);
	  	if ("vibrate" in navigator) {
	  	navigator.vibrate(50);
	  	}
    });
    
    $("#game_left").on("tap", function() {
	  	snake_setdirection(1);
	  	tetris_setdirection(1);
	  	if ("vibrate" in navigator) {
	  	navigator.vibrate(50);
	  	}
    });
    
    
    $("#game_down").on("tap", function() {
	  	snake_setdirection(2);
	  	tetris_setdirection(2);
	  	if ("vibrate" in navigator) {
	  	navigator.vibrate(50);
	  	}
    });
    
    $("#console_output").change(function() {
        var val = ($(this).val() == "on" ? 1 : 0);
        sendDataU8(FRAMECOMMAND.FRC_CONSOLE_ENABLE, parseInt(val));
    });
    $("#fade").change(function() {
        //console.log("change" + $(this).val());
        var val = ($(this).val() == "on" ? 1 : 0);
        sendDataU8(FRAMECOMMAND.FRC_FADE_ENABLE, parseInt(val));
    });
    
    $("#clientconnect").change(function() {
        var val =  ($(this).val() == "on" ? "1" : "0");
        sendDataU8(FRAMECOMMAND.FRC_CONNECT_CLIENT_SAVE, val);
    });
    
    
     $("#font-color").hexColorPicker();
    
     $("#random_choise").change(function () {
          var str = "";
          $("#random_choise option:selected").each(function () {
             str += $(this).val() + ",";
          });
          //ws.send("RANC:," + str);
          sendDataString(FRAMECOMMAND.FRC_SLIDESHOW_CHOISE, "," + str);
      });
        
    WebSocketTest();
    MakeDragDrop( "header", DragDropSystemFiles );
   // snake_init();
    
    var y = 0;
    var x = 0;
    var container = $("#ledscreen");

var lookupscreen2 = 	[	14,13,12,11,10,9,8,7,6,5,4,3,2,1,0,
						15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,
						44,43,42,41,40,39,38,37,36,35,34,33,32,31,30,
						45,46,47,48,49,50,51,52,53,54,55,56,57,58,59,
						74,73,72,71,70,69,68,67,66,65,64,63,62,61,60,
						75,76,77,78,79,80,81,82,83,84,85,86,87,88,89,
						104, 103,102,101,100,99,98,97,96,95,94,93,92,91,90,
						105,106,107,108,109,110,111,112,113,114,115,116,117,118,119,
						];
						
var lookupscreen = 	[ 0,	8,	16,	24,	32,	40,	48,	56,	64,	72,	80,	88,	96,	104,	112,
1,	9,	17,	25,	33,	41,	49,	57,	65,	73,	81,	89,	97,	105,	113,
2,	10,	18,	26,	34,	42,	50,	58,	66,	74,	82,	90,	98,	106,	114,
3,	11,	19,	27,	35,	43,	51,	59,	67,	75,	83,	91,	99,	107,	115,
4,	12,	20,	28,	36,	44,	52,	60,	68,	76,	84,	92,	100,	108,	116,
5,	13,	21,	29,	37,	45,	53,	61,	69,	77,	85,	93,	101,	109,	117,
6,	14,	22,	30,	38,	46,	54,	62,	70,	78,	86,	94,	102,	110,	118,
7,	15,	23,	31,	39,	47,	55,	63,	71,	79,	87,	95,	103,	111,	119];
						

    var ledcount = 0;
    var html = "";
    for(x = 7; x >= 0; x--) {
    	html += "<div class=container>";
    	for(y = 14; y >= 0; y--) {
    		ledcount = (x*15) + y;
    		html+="<div id=\"led" + lookupscreen[ledcount] + "\" class=\"grid gridblock\"></div>";
    		ledcount++;
    	}
    	html+="</div>";
    }
    container.append(html);
});