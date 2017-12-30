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


    var ws = null;
    var z;
    var c ;            
    var graph;
	var xPadding = 0;
	var yPadding = 0;
    var alive = 0;
    var count = 0;
    setInterval(function(){ 
        if(count > 5) {
        	ws.send("PING");
            count = 0;
        }
        
        if(alive > 10 && ws.readyState == 1) {
             ws.close();
             ws = null;
             WebSocketTest();
             alive = 0;
        }
        
        count++;
        alive++;
    }, 1000);


     function WebSocketTest()
     {
        if ("WebSocket" in window)
        {
           if(ws == null) {
                ws = new WebSocket("ws://84.31.4.171:8000/echo");
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
                    ws.send("Opened connection");
               };
			
               ws.onmessage = function (evt) 
               { 
                   alive = 0
                  var received_msg = evt.data;
                  var myarr = received_msg.split(":");
                  switch(myarr[0]) {
                    case "BRI":
                        receiveBrightness(myarr[1]);
                     break;
                    case "GIF":
                        receiveAnimation(myarr[1]);
                     break;       
                    case "ASP":
                        receiveAnimationSpeed(myarr[1]);
                        break;
                    case "FLI":
                        receiveFlicker(myarr[1]);
                        break;
                    case "FLS":
                        receiveFlickerSpeed(myarr[1]);
                        break;
                    case "FAS":
                        receiveFadeSpeed(myarr[1]);
                        break;
                    case "FAD":
                        receiveFade(myarr[1]);
                        break;
                    case "TEXT":
                        receiveText(myarr[1]);
                        break;
                    case "TSP":
                        receiveTextSpeed(myarr[1]);
                        break;
                    default:
                     break;                                  
                  }
                  console.log(received_msg);
               };
			
		       ws.onerror = function(event)
		       {
		                ws.close();
		            ws = null;
        			WebSocketTest();
		       }
               ws.onclose = function(event)
               { 
        			//alert(event.code);
        					            ws.close();
        			ws = null;
        			toast("Connecting...", true);
        			WebSocketTest();
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
        if(dat != null) {
           dat.fadeOut(400, function(){
	            $(this).remove();
            });
        }
    
      if(dat2 != null) {
       dat2.fadeOut(400, function(){
	        $(this).remove();
        });
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

    function sendBrightness() {
        ws.send("BRI:" + $("#brightness").val());
    }
    function receiveBrightness(data) {
        $("#brightness").val(data).trigger('create').slider("refresh");;
    }
	function sendAnimation() {
        ws.send("GIF:" + (($("#animation").val() * 1) - 1));
    }
    function receiveAnimation(data) {
        $("#animation").val(((parseInt(data)) + 1)).selectmenu("refresh", true);
    }
    function sendAnimationSpeed() {
        ws.send("ASP:" + (parseInt($("#animation_speed").attr("max")) - parseInt($("#animation_speed").val()) + 1));
    }
    function receiveAnimationSpeed(data) {
        console.log("SS" + (parseInt($("#animation_speed").attr("max")) - data + 1));
        $("#animation_speed").val((parseInt($("#animation_speed").attr("max")) - data + 1)).trigger('create').slider("refresh");
    }
    function receiveFlicker(data) {
        console.log(data);
        $("#flicker").val(data == "1" ? "on" : "off").trigger('create').slider("refresh");
    }
    function receiveFade(data) {
        console.log(data);
        $("#fade").val(data == "1" ? "on" : "off").trigger('create').slider("refresh");

    }
    function receiveFlickerSpeed(data) {
        console.log(data + "WW");
        $("#flicker_speed").val(data).trigger('create').slider("refresh");
    }
    function receiveFadeSpeed(data) {
        console.log(data + "WW");
        $("#fade_speed").val(data).trigger('create').slider("refresh");
    }
    function receiveTextSpeed(data) {
        console.log("TSS" + (parseInt($("#text_speed").attr("max")) - data + 1));
        $("#text_speed").val((parseInt($("#text_speed").attr("max")) - data + 1)).trigger('create').slider("refresh");
    }
    function receiveText(data) {
        $("#input_text").val(data);
    }
    function sendText(data) {
        ws.send("TEXT:" + data + " ");
    }
     
            
 $(document).on("pagecreate","#page1",function(){

        $.mobile.changePage.defaults.changeHash = false;
        $.mobile.hashListeningEnabled = false;
        $.mobile.pushStateEnabled = false;
        var browser = navigator.sayswho.toLowerCase();
	    $('body').addClass(browser);
	
        $("#brightness").change(function() {
           sendBrightness();
        });
        $("#animation").change(function() {
            sendAnimation();
        });
        $("#animation_speed").change(function() {
            sendAnimationSpeed();
        });
        $("#text_speed").change(function() {
            console.log((parseInt($(this).attr("max")) + 1 - parseInt($(this).val())));
            ws.send("TSP:" + (parseInt($(this).attr("max")) + 1 - parseInt($(this).val())));
        });
        $("#button_send").on("tap", function() {
            sendText($("#input_text").val());
        });
        $("#flicker_speed").change(function() {
            ws.send("FLS:" + $("#flicker_speed").val());
        });
        $("#fade_speed").change(function() {
            console.log("change");
            ws.send("FAS:" + $("#fade_speed").val());
        });
        $("#flicker").change(function() {
            console.log("change" + $(this).val());
            ws.send("FLI:" + ($(this).val() == "on" ? "1" : "0"));
        });
        $("#fade").change(function() {
            console.log("change" + $(this).val());
            ws.send("FAD:" + ($(this).val() == "on" ? "1" : "0"));
        });
        WebSocketTest();
        console.log("Done");
    });
    
    function setGif(dat) {
    	ws.send("GIF:" + dat);
    }
