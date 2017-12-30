

var output;
var websocket;
var commsup = 0;

var mpfs_start_at = 1048576;
var flash_scratchpad_at = 524288;
var flash_blocksize = 65536;
var flash_sendsize = 1024;
//Push objects that have:
// .request
// .callback = function( ref (this object), data );

var workqueue = [];
var wifilines = [];
var workarray = {};
var lastitem;
var files;
var md51;
var md52;
var size1;
var size2;

var myVar = setInterval(function(){ lastitemtime() }, 500);

function myStopFunction() {
    clearInterval(myVar);
}

function lastitemtime() {
  if( lastitem && lastitem  != 0)
  {
  		lastitem.timer += 1;
        if( lastitem.timer > 5 && lastitem.callback )
        {
        	console.log("timeout");
            lastitem.callback( lastitem, "MD5:1" );
        }
    }
}

function flash_message_callback(evt) {
 if( lastitem && lastitem  != 0)
  {
        if( lastitem.callback )
        {
            lastitem.callback( lastitem, evt.data );
        }
    }
}

function _arrayBufferToString(buf, callback) {
    var bb = new Blob([new Uint8Array(buf)]);
    
    var f = new FileReader();
    f.onload = function(e) {
        callback(e.target.result);
    };
    f.readAsText(bb);
}
// string to uint array
function unicodeStringToTypedArray(s) {
    var escstr = encodeURIComponent(s);
    var binstr = escstr.replace(/%([0-9A-F]{2})/g, function(match, p1) {
        return String.fromCharCode('0x' + p1);
    });
    var ua = new Uint8Array(binstr.length);
    Array.prototype.forEach.call(binstr, function (ch, i) {
        ua[i] = ch.charCodeAt(0);
    });
    return ua;
}


function appendBuffer( buffer1, buffer2 ) {
  var tmp = new Uint8Array( buffer1.byteLength + buffer2.byteLength );
  tmp.set( new Uint8Array( buffer1 ), 0 );
  tmp.set( new Uint8Array( buffer2 ), buffer1.byteLength );
  return tmp.buffer;
}
var file1 = null;
var file2 = null;
var countertry=0;
function sendFileChunks(arraydata, pushop, filenum, filetot, file ) 
{
    console.log("Going to send " +  pushop.padlen);
	countertry = 0;
    if( pushop.place >= pushop.padlen ) {
        console.log("Done sending!");
        
        if(filenum != filetot-1) {
        	console.log("Sending next file");
            sendFile(file2, 0x40000, filenum+1, filetot);
        } else {
        	if(filetot>1) {
	       		var action = Object();
			    var u32 = new Uint32Array(1, 0x80000);
			    var sendstr = new ArrayBuffer(520); // 256-byte ArrayBuffer.
			    var dv = new DataView(sendstr);
			    dv.setUint8(0, 0x46);
			    dv.setUint8(1, 0x44);
			    dv.setUint8(2, 0x35);
			    dv.setUint8(3, 0x3a);    
			    dv.setUint32(4, 0x00080000, true);    
			    dv.setUint32(8, 0x00000000, true); 
			    dv.setUint32(12, size1, true);  
			    for(var i = 16; i < 16 + 48; i++) {
			        dv.setUint8(i, md51.charCodeAt(i-16));  
			    }      
			 
			    dv.setUint32(64, 0x000c0000, true);    
			    dv.setUint32(68, 0x00040000, true); 
			    dv.setUint32(72, size2, true);  
			    for(var i = 76; i < 76 + 48; i++) {
			        dv.setUint8(i, md52.charCodeAt(i-76));  
			    }  
		    	var bb = new Blob([new Uint8Array(sendstr)]);
		        ws.binaryType = "blob";
		        action.callback = function(item, data) {
		        	item.timer = 0;
		        	lastitem = null;  
		        	 console.log(data);
		            //alert(data);      
		        	//location.reload();
		        };
		        action.timer = 0;
		        lastitem  = action;
		        var p = ws.send(bb);
		        if(p == 0) {
		        	alert("Done flashing data");
		        	console.log("Done flashing data");
		        	location.reload();
		        }
		        return;
	        } else {
	        	var newaddr = 0x150000;
	        	
        		if( file.name.substr( 0, 3 ) == "gif" ) { 
					newaddr = 0x100000; // 0x100000 -   0x80000 = 0x80000;
				}
				
				if( file.name.substr( 0, 3 ) == "web" ) { 
					newaddr = 0x150000; // 0x150000 -   0x80000 = 0x80000;
				}
				
	        	var action = Object();
			    var u32 = new Uint32Array(1, 0x80000);
			    var sendstr = new ArrayBuffer(520); // 256-byte ArrayBuffer.
			    var dv = new DataView(sendstr);
			    dv.setUint8(0, 0x46);
			    dv.setUint8(1, 0x44);
			    dv.setUint8(2, 0x35);
			    dv.setUint8(3, 0x3a);    
			    dv.setUint32(4, 0x00080000, true);    
			    dv.setUint32(8, newaddr, true); 
			    dv.setUint32(12, size1, true);  
			    for(var i = 16; i < 16 + 48; i++) {
			        dv.setUint8(i, md51.charCodeAt(i-16));  
			    }      
			 
			    dv.setUint32(64, 0x000c0000, true);    
			    dv.setUint32(68, 0x00040000, true); 
			    dv.setUint32(72, 0, true);  
			    for(var i = 76; i < 76 + 48; i++) {
			        dv.setUint8(i, 0);  
			    }  
		    	var bb = new Blob([new Uint8Array(sendstr)]);
		        ws.binaryType = "blob";
		        
		        action.callback = function(item, data) {
		        	item.timer = 0;  
		        	 console.log(data);      
		        	if(data.indexOf("ASP:") >= 0) {	
		        		countertry++;
		        	}
		        	if(countertry > 3) {
		        		location.reload();
		        		alert("Try again");
		        	}
		        	if(data.indexOf("STATUS:") >= 0) {
		        		alert("status:" + data);
		        		location.reload();
		        	}
		        };
		        action.timer = 0;
		        lastitem  = action;
		        var p = ws.send(bb);
		        
		        if(p == 0) {
		        alert("Done flashing data");
		        console.log("Done flashing data");
		        location.reload();
		        }
	        	return;
	        }
	    }
   	}

 	console.log("Going to send 2 " +  pushop.padlen);

    var addy = pushop.place + pushop.base_address;
    pushop.place + pushop.base_address;
       
    var sendstr = "FLB:" + flash_sendsize + ":" + pushop.place + ":" + pushop.base_address + ":";

    var action = Object();
    action.pushop = pushop;
    
    
    ws.binaryType = "blob";
   
    var ua = unicodeStringToTypedArray(sendstr);
    var tmp = new Uint8Array( sendstr.length + flash_sendsize, 0 );
    var tmpmd = new Uint8Array( flash_sendsize, 0 );
    tmp.set(ua, 0);
    tmp.set(pushop.paddata.slice(pushop.place, pushop.place + flash_sendsize), sendstr.length);
    tmpmd.set(pushop.paddata.slice(pushop.place, pushop.place + flash_sendsize), 0);
    var md5temp = faultylabs.MD5( tmpmd ).toLowerCase();
  	
    var trycount = 0;
        
    action.callback = function(item, data) {  
    	var p = item.pushop;
    	item.timer = 0;
    	trycount++;
    	var data2 = data.split(":");   
    	if(data2[0] == "MD5" && data2[1] == md5temp) { 
    			p.place += flash_sendsize;
    			sendFileChunks(arraydata, p, filenum, filetot, file);
    		} else if(data2[0] == "MD5") { 
    			console.log("ERROR -" + data + "-" + data2[0] + "-" + md5temp); 
    			//if(trycount > 10) {
    				console.log("Error flasing data");
    				//p.place -= flash_sendsize;
    				sendFileChunks(arraydata, p, filenum, filetot, file);
    				trycount = 0;
    				//lastitem = null;
    				
    			//}  
    		} 
      };
      action.timer = 0;
    lastitem  = action;
    ws.send(tmp);
    console.log("Done sending " +  pushop.padlen + sendstr);
        
}

function sendFile(file, offset, filenum, filetot) {
	//alert("Starting flash. Please wait!");
	
    var reader = new FileReader();
    reader.onload = function(e) { 
	    var arraydata = e.target.result;
	    var padlen = Math.floor(((arraydata.byteLength-1)/flash_sendsize)+1)*flash_sendsize;
	    var paddata = new Uint8Array( padlen, 0 );
	    paddata.set( new Uint8Array( arraydata ), 0 );
	                        
	    if(filenum == 0) {
	       size1 = padlen;
	       md51 = faultylabs.MD5( paddata ).toLowerCase();
	    }else{
	       size2 = padlen;
	       md52= faultylabs.MD5( paddata ).toLowerCase();
	    }
			
        var action = Object();
        var address = flash_scratchpad_at + offset;
        var fromsector = (address / 0x1000);
        var tosector = padlen / 0x1000;
        var sendstr = "FLE:" + fromsector + ":" + tosector +":"; 
        console.log("ERASE message" + sendstr); 
        action.callback = function(item, data) {
        	item.timer = 0; 
            lastitem = null;  
            var pushop = Object();
            pushop.place = 0;
            pushop.padlen = Math.floor(((arraydata.byteLength-1)/flash_sendsize)+1)*flash_sendsize;
            pushop.paddata = new Uint8Array( pushop.padlen, 0 );
            pushop.paddata.set( new Uint8Array( arraydata ), 0 );
            pushop.base_address = flash_scratchpad_at + offset;
            sendFileChunks(paddata, pushop, filenum,filetot, file);
        };
    	action.timer = 0;
        lastitem  = action;
        ws.send(sendstr);
    }
    reader.readAsArrayBuffer( file );
    return;
}


function DragDropSystemFiles( file )
{

	toast("Flashing... Please wait", true);
	set_flashing(1);
	if( file.length == 1 )
	{
		var offset = 0;
		
		for( var i = 0; i < file.length; i++ )
		{
			if( file[i].name.substr( 0, 3 ) == "web" ) { 
				file1 = file[i]; 
				offset = 0x00000; // 0x150000 (place for html) -   0x80000 (scatchpad)
			}
			
			if( file[i].name.substr( 0, 3 ) == "gif" ) { 
				file1 = file[i]; 
				offset = 0x00000; // 0x100000 -   0x80000 = 0x80000;
			}
		}

		if( !file1 )
		{
			
			alert( "Could not find a web... file." ); return;
		}

		if(  file1.size > 150000 )
		{
			alert( "web needs to fit in IRAM.  Too big." + file1.size ); return;
		}

        files = file;
		//Files check out.  Start pushing.
        sendFile(file1, offset, 0, 1);  
	}
	else if( file.length == 2 )
	{
		
		for( var i = 0; i < file.length; i++ )
		{
			if( file[i].name.substr( 0, 7 ) == "0x00000" ) file1 = file[i];
			if( file[i].name.substr( 0, 7 ) == "0x40000" ) file2 = file[i];
		}

		if( !file1 )
		{
		
			alert( "Could not find a 0x00000... file." ); return;
		}

		if( !file2 )
		{
			alert( "Could not find a 0x40000... file." ); return;
		}

		if(  file1.size > 65536 )
		{
			alert( "0x00000 needs to fit in IRAM.  Too big." ); return;
		}

		if(  file2.size > 262144 )
		{
			alert( "0x40000 needs to fit in 256kB.  Too big." ); return;
		}
        files = file;
		//Files check out.  Start pushing.
        sendFile(file1, 0x00000, 0, 2);
		
	}
	else
	{
		alert("Not accepting files.");
		//==$("#innersystemflashtext").html( "Cannot accept anything other than 1 or 2 files." );
	}
}




function MakeDragDrop( divname, callback )
{
	var obj = $("#" + divname);
	obj.width("100%");
	obj.height("40px");
	obj.on('dragenter', function (e) 
	{
		e.stopPropagation();
		e.preventDefault();
		$(this).css('border', '2px solid #0B85A1');
	});

	obj.on('dragover', function (e) 
	{
		e.stopPropagation();
		e.preventDefault();
	});

	obj.on('dragend', function (e) 
	{
		e.stopPropagation();
		e.preventDefault();
		$(this).css('border', '2px dotted #0B85A1');
	});

	obj.on('drop', function (e) 
	{
		$(this).css('border', '2px dotted #0B85A1');
		e.preventDefault();
		var files = e.originalEvent.dataTransfer.files;

		//We need to send dropped files to Server
		callback(files);
	});
}



/* MD5 implementation minified from: http://blog.faultylabs.com/files/md5.js
 Javascript MD5 library - version 0.4 Coded (2011) by Luigi Galli - LG@4e71.org - http://faultylabs.com
 Thanks to: Roberto Viola  The below code is PUBLIC DOMAIN - NO WARRANTY!
 */
"undefined"==typeof faultylabs&&(faultylabs={}),faultylabs.MD5=function(n){function r(n){var r=(n>>>0).toString(16);return"00000000".substr(0,8-r.length)+r}function t(n){for(var r=[],t=0;t<n.length;t++)r=r.concat(s(n[t]));return r}function e(n){for(var r=[],t=0;8>t;t++)r.push(255&n),n>>>=8;return r}function o(n,r){return n<<r&4294967295|n>>>32-r}function a(n,r,t){return n&r|~n&t}function f(n,r,t){return t&n|~t&r}function u(n,r,t){return n^r^t}function i(n,r,t){return r^(n|~t)}function c(n,r){return n[r+3]<<24|n[r+2]<<16|n[r+1]<<8|n[r]}function s(n){for(var r=[],t=0;t<n.length;t++)if(n.charCodeAt(t)<=127)r.push(n.charCodeAt(t));else for(var e=encodeURIComponent(n.charAt(t)).substr(1).split("%"),o=0;o<e.length;o++)r.push(parseInt(e[o],16));return r}function l(){for(var n="",t=0,e=0,o=3;o>=0;o--)e=arguments[o],t=255&e,e>>>=8,t<<=8,t|=255&e,e>>>=8,t<<=8,t|=255&e,e>>>=8,t<<=8,t|=e,n+=r(t);return n}function y(n){for(var r=new Array(n.length),t=0;t<n.length;t++)r[t]=n[t];return r}function h(n,r){return 4294967295&n+r}function p(){function n(n,r,t,e){var a=m;m=U,U=d,d=h(d,o(h(b,h(n,h(r,t))),e)),b=a}var r=A.length;A.push(128);var t=A.length%64;if(t>56){for(var s=0;64-t>s;s++)A.push(0);t=A.length%64}for(s=0;56-t>s;s++)A.push(0);A=A.concat(e(8*r));var y=1732584193,p=4023233417,g=2562383102,v=271733878,b=0,d=0,U=0,m=0;for(s=0;s<A.length/64;s++){b=y,d=p,U=g,m=v;var I=64*s;n(a(d,U,m),3614090360,c(A,I),7),n(a(d,U,m),3905402710,c(A,I+4),12),n(a(d,U,m),606105819,c(A,I+8),17),n(a(d,U,m),3250441966,c(A,I+12),22),n(a(d,U,m),4118548399,c(A,I+16),7),n(a(d,U,m),1200080426,c(A,I+20),12),n(a(d,U,m),2821735955,c(A,I+24),17),n(a(d,U,m),4249261313,c(A,I+28),22),n(a(d,U,m),1770035416,c(A,I+32),7),n(a(d,U,m),2336552879,c(A,I+36),12),n(a(d,U,m),4294925233,c(A,I+40),17),n(a(d,U,m),2304563134,c(A,I+44),22),n(a(d,U,m),1804603682,c(A,I+48),7),n(a(d,U,m),4254626195,c(A,I+52),12),n(a(d,U,m),2792965006,c(A,I+56),17),n(a(d,U,m),1236535329,c(A,I+60),22),n(f(d,U,m),4129170786,c(A,I+4),5),n(f(d,U,m),3225465664,c(A,I+24),9),n(f(d,U,m),643717713,c(A,I+44),14),n(f(d,U,m),3921069994,c(A,I),20),n(f(d,U,m),3593408605,c(A,I+20),5),n(f(d,U,m),38016083,c(A,I+40),9),n(f(d,U,m),3634488961,c(A,I+60),14),n(f(d,U,m),3889429448,c(A,I+16),20),n(f(d,U,m),568446438,c(A,I+36),5),n(f(d,U,m),3275163606,c(A,I+56),9),n(f(d,U,m),4107603335,c(A,I+12),14),n(f(d,U,m),1163531501,c(A,I+32),20),n(f(d,U,m),2850285829,c(A,I+52),5),n(f(d,U,m),4243563512,c(A,I+8),9),n(f(d,U,m),1735328473,c(A,I+28),14),n(f(d,U,m),2368359562,c(A,I+48),20),n(u(d,U,m),4294588738,c(A,I+20),4),n(u(d,U,m),2272392833,c(A,I+32),11),n(u(d,U,m),1839030562,c(A,I+44),16),n(u(d,U,m),4259657740,c(A,I+56),23),n(u(d,U,m),2763975236,c(A,I+4),4),n(u(d,U,m),1272893353,c(A,I+16),11),n(u(d,U,m),4139469664,c(A,I+28),16),n(u(d,U,m),3200236656,c(A,I+40),23),n(u(d,U,m),681279174,c(A,I+52),4),n(u(d,U,m),3936430074,c(A,I),11),n(u(d,U,m),3572445317,c(A,I+12),16),n(u(d,U,m),76029189,c(A,I+24),23),n(u(d,U,m),3654602809,c(A,I+36),4),n(u(d,U,m),3873151461,c(A,I+48),11),n(u(d,U,m),530742520,c(A,I+60),16),n(u(d,U,m),3299628645,c(A,I+8),23),n(i(d,U,m),4096336452,c(A,I),6),n(i(d,U,m),1126891415,c(A,I+28),10),n(i(d,U,m),2878612391,c(A,I+56),15),n(i(d,U,m),4237533241,c(A,I+20),21),n(i(d,U,m),1700485571,c(A,I+48),6),n(i(d,U,m),2399980690,c(A,I+12),10),n(i(d,U,m),4293915773,c(A,I+40),15),n(i(d,U,m),2240044497,c(A,I+4),21),n(i(d,U,m),1873313359,c(A,I+32),6),n(i(d,U,m),4264355552,c(A,I+60),10),n(i(d,U,m),2734768916,c(A,I+24),15),n(i(d,U,m),1309151649,c(A,I+52),21),n(i(d,U,m),4149444226,c(A,I+16),6),n(i(d,U,m),3174756917,c(A,I+44),10),n(i(d,U,m),718787259,c(A,I+8),15),n(i(d,U,m),3951481745,c(A,I+36),21),y=h(y,b),p=h(p,d),g=h(g,U),v=h(v,m)}return l(v,g,p,y).toUpperCase()}var A=null,g=null;return"string"==typeof n?A=s(n):n.constructor==Array?0===n.length?A=n:"string"==typeof n[0]?A=t(n):"number"==typeof n[0]?A=n:g=typeof n[0]:"undefined"!=typeof ArrayBuffer?n instanceof ArrayBuffer?A=y(new Uint8Array(n)):n instanceof Uint8Array||n instanceof Int8Array?A=y(n):n instanceof Uint32Array||n instanceof Int32Array||n instanceof Uint16Array||n instanceof Int16Array||n instanceof Float32Array||n instanceof Float64Array?A=y(new Uint8Array(n.buffer)):g=typeof n:g=typeof n,g&&alert("MD5 type mismatch, cannot process "+g),p()};
        