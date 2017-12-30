var tetristimer = null;
var tetris_playfield;
var tetris_map;
var total_rows = 8;
var total_colums = maxcolumns;


function tetris_drawplayfield() {
  var count = 0;
  for(var x = 0; x < total_colums;  x++) {
    for(var y = 0; y < 8;  y++) {
	    var clr = tetris_map[x][y];
	    if(clr == 1) {
		    tetris_playfield[count] = 0xFF;
		    tetris_playfield[count+1] = 0x00;
		    tetris_playfield[count+2] = 0x00;
	    } else if(clr == 3) {
		    tetris_playfield[count] = 0x00;
		    tetris_playfield[count+1] = 0xFF;
		    tetris_playfield[count+2] = 0xFF;
	    } else if(clr == 4) {
		    tetris_playfield[count] = 0x00;
		    tetris_playfield[count+1] = 0x00;
		    tetris_playfield[count+2] = 0xFF;
	    } else if(clr == 5) {
		    tetris_playfield[count] = 0xFF;
		    tetris_playfield[count+1] = 0x66;
		    tetris_playfield[count+2] = 0x00;
	    } else if(clr == 6) {
		    tetris_playfield[count] = 0xFF;
		    tetris_playfield[count+1] = 0xFF;
		    tetris_playfield[count+2] = 0x00;
	    }  else if(clr == 7) {
		    tetris_playfield[count] = 0x00;
		    tetris_playfield[count+1] = 0xFF;
		    tetris_playfield[count+2] = 0x00;
	    } else if(clr == 8) {
		    tetris_playfield[count] = 0x80;
		    tetris_playfield[count+1] = 0x00;
		    tetris_playfield[count+2] = 0x80;
	    }
	     else if(clr == 9) {
		    tetris_playfield[count] = 0xFF;
		    tetris_playfield[count+1] = 0x00;
		    tetris_playfield[count+2] = 0x00;
	    } else {
		    tetris_playfield[count] = 0x00;
		    tetris_playfield[count+1] = 0x00;
		    tetris_playfield[count+2] = 0x00;
	    }
	    count+=3;
     }
   }
}


    function handle(action) {
      switch(action) {
        case DIR.LEFT:  rotate();  break;
        case DIR.RIGHT: drop();    break;
        case DIR.UP:    move(DIR.UP);        break;
        case DIR.DOWN:  move(DIR.DOWN);          break;
      }
    }


function update(idt) {
  if (playing) {
    
    handle(actions.shift());

    drop();
    
  }
}

function tetris_frametimer() {

    for(var x = 0; x < total_colums;  x++) {
      for(var y = 0; y < 8;  y++) {
	       tetris_map[x][y] = 0;
	    }
	}

    update(0);
    drawCourt();
    tetris_drawplayfield();
   

    send_leddata(tetris_playfield);
	

}

function tetris_reset() {
  next = 0;
  clearActions();
  clearBlocks();
  setCurrentPiece(next);
  setNextPiece();
  
  
  playing = true;
}

function tetris_stop() {
	if(tetristimer != null) {
		clearInterval(tetristimer);
		tetristimer = null;
	}
}

function tetris_init() {
	total_colums = get_columns();
	nx = total_colums;
	if(tetristimer != null)
		clearInterval(tetristimer);
	
	tetris_playfield = new Array();
	var count = 0;
	 for(var x = 0; x < total_colums;  x++) {
	    	for(var y = 0; y < 8;  y++) {
	    		tetris_playfield[count] = 0x00;
				tetris_playfield[count+1] = 0x00;
				tetris_playfield[count+2] = 0x00;
				count+=3;
	    	}
	  }
	  
	tetris_map = new Array(total_colums);
    for (var i = 0; i < tetris_map.length; i++) {
        tetris_map[i] = new Array(8);
    }
    
    tetris_reset();
    
  
	tetristimer = setInterval(tetris_frametimer, 400 );	
}


function drawCourt() {

    if (playing)
      drawPiece(current.type, current.x, current.y, current.dir);
      
      var x, y, block;
        for(y = 0 ; y < ny ; y++) {
          for (x = 0 ; x < nx ; x++) {
            if (block = getBlock(x,y))
              drawBlock(x, y, block.color);
          }
        }
        
}
  
function drawBlock(x, y, col) {
     tetris_map[x][y] = col;
     
     //      ctx.fillRect(x*dx, y*dy, dx, dy);
     // ctx.strokeRect(x*dx, y*dy, dx, dy)
      
}
    
function drawPiece(type, x, y, dir) {
  eachblock(type, x, y, dir, function(x, y) {
     tetris_map[x][y] = type.color;
  });
}


function timestamp()           { return new Date().getTime();                             }
function random(min, max)      { return (min + (Math.random() * (max - min)));            }
function randomChoice(choices) { return choices[Math.round(random(0, choices.length-1))]; }

function clearScore()           { setScore(0); }
function clearRows()            { setRows(0); }
function setRows(n)             { rows = n; step = Math.max(speed.min, speed.start - (speed.decrement*rows));  }
function addRows(n)             { setRows(rows + n); }
function getBlock(x,y)          { return (blocks && blocks[x] ? blocks[x][y] : null); }
function setBlock(x,y,type)     { blocks[x] = blocks[x] || []; blocks[x][y] = type; }
function clearBlocks()          { blocks = []; }
function clearActions()         { actions = []; }
function setCurrentPiece(piece) { current = piece || randomPiece();      }
function setNextPiece(piece)    { next    = piece || randomPiece();  }


var KEY     = { ESC: 27, SPACE: 32, LEFT: 37, UP: 38, RIGHT: 39, DOWN: 40 },
    DIR     = { UP: 0, RIGHT: 1, DOWN: 2, LEFT: 3, MIN: 0, MAX: 3 },
    speed   = { start: 0.6, decrement: 0.005, min: 0.1 }, // how long before piece drops by 1 row (seconds)
    nx      = total_colums, // width of tetris court (in blocks)
    ny      = 8; // height of tetris court (in blocks)
    
    
//-------------------------------------------------------------------------
// game variables (initialized during reset)
//-------------------------------------------------------------------------
var dx, dy,        // pixel size of a single tetris block
    blocks,        // 2 dimensional array (nx*ny) representing tetris court - either empty block or occupied by a 'piece'
    actions,       // queue of user actions (inputs)
    playing,       // true|false - game is in progress
    dt,            // time since starting this game
    current,       // the current piece
    next,          // the next piece
    score,         // the current score
    vscore,        // the currently displayed score (it catches up to score in small chunks - like a spinning slot machine)
    rows,          // number of completed rows in the current game
    step;          // how long before current piece drops by 1 row

//-------------------------------------------------------------------------
// tetris pieces
//
// blocks: each element represents a rotation of the piece (0, 90, 180, 270)
//         each element is a 16 bit integer where the 16 bits represent
//         a 4x4 set of blocks, e.g. j.blocks[0] = 0x44C0
//
//             0100 = 0x4 << 3 = 0x4000
//             0100 = 0x4 << 2 = 0x0400
//             1100 = 0xC << 1 = 0x00C0
//             0000 = 0x0 << 0 = 0x0000
//                               ------
//                               0x44C0
//
//-------------------------------------------------------------------------
var i = { size: 4, blocks: [0x0F00, 0x2222, 0x00F0, 0x4444], color: 3}; //  'cyan'   };
var j = { size: 2, blocks: [0x44C0, 0x8E00, 0x6440, 0x0E20], color: 4}; //'blue'   };
var l = { size: 2, blocks: [0x4460, 0x0E80, 0xC440, 0x2E00], color: 5}; //'orange' };
var o = { size: 2, blocks: [0xCC00, 0xCC00, 0xCC00, 0xCC00], color: 6}; //'yellow' };
var s = { size: 3, blocks: [0x06C0, 0x8C40, 0x6C00, 0x4620], color: 7}; //'green'  };
var t = { size: 3, blocks: [0x0E40, 0x4C40, 0x4E00, 0x4640], color: 8}; //'purple' };
var z = { size: 3, blocks: [0x0C60, 0x4C80, 0xC600, 0x2640], color: 9}; //'red'    };
//------------------------------------------------
// do the bit manipulation and iterate through each
// occupied block (x,y) for a given piece
//------------------------------------------------
function eachblock(type, x, y, dir, fn) {
  var bit, result, row = 0, col = 0, blocks = type.blocks[dir];
  for(bit = 0x8000 ; bit > 0 ; bit = bit >> 1) {
    if (blocks & bit) {
      fn(x + col, y + row);
    }
    if (++col === 4) {
      col = 0;
      ++row;
    }
  }
}
//-----------------------------------------------------
// check if a piece can fit into a position in the grid
//-----------------------------------------------------
function occupied(type, x, y, dir) {
  var result = false
  eachblock(type, x, y, dir, function(x, y) {
    if ((x < 0) || (x >= nx) || (y < 0) || (y >= ny) || getBlock(x,y))
      result = true;
  });
  return result;
}

function unoccupied(type, x, y, dir) {
  return !occupied(type, x, y, dir);
}
//-----------------------------------------
// start with 4 instances of each piece and
// pick randomly until the 'bag is empty'
//-----------------------------------------
var pieces = [];
function randomPiece() {
  if (pieces.length == 0)
    pieces = [i,i,i,i,j,j,j,j,l,l,l,l,o,o,o,o,s,s,s,s,t,t,t,t,z,z,z,z];
    
    var type = pieces.splice(random(0, pieces.length-1), 1)[0];
  return { type: type, dir: DIR.RIGHT, x: 0, y: Math.round(random(0, ny - type.size)) };
}



//tetris_setdirection

    function tetris_setdirection(ev) {
    	if(tetristimer == null)
		return;
		
      var handled = false;
      if (playing) {
        switch(ev) {
          case 1:   actions.push(DIR.LEFT);  handled = true; break;
          case 0:   actions.push(DIR.RIGHT); handled = true; break;
          case 3:   actions.push(DIR.UP);    handled = true; break;
          case 2:   actions.push(DIR.DOWN);  handled = true; break;
          case 4:    lose();                  handled = true; break;
        }
      }
		  
		for(var x = 0; x < total_colums;  x++) {
		  for(var y = 0; y < 8;  y++) {
		       tetris_map[x][y] = 0;
		    }
		}
		handle(actions.shift());
		drawCourt();
		tetris_drawplayfield();
		send_leddata(tetris_playfield);
    }



    function move(dir) {
      var x = current.x, y = current.y;
      switch(dir) {
        case DIR.RIGHT: x = x + 1; break;
        case DIR.LEFT:  x = x - 1; break;
        case DIR.DOWN:  y = y - 1; break;
        case DIR.UP:    y = y + 1; break;
      }
      if (unoccupied(current.type, x, y, current.dir)) {
        current.x = x;
        current.y = y;
         //console.log("moving block");
        return true;
      }
      else {
        //console.log("COLLIDING block");
        return false;
      }
    }
    
    function rotate() {
      var newdir = (current.dir == DIR.MAX ? DIR.MIN : current.dir + 1);
      if (unoccupied(current.type, current.x, current.y, newdir)) {
        current.dir = newdir;
        invalidate();
      }
    }
    
    function drop() {
      if (!move(DIR.RIGHT)) {
        // addScore(10);
        dropPiece();
        removeLines();
        setCurrentPiece(next);
        setNextPiece(randomPiece());
        clearActions();
        //console.log("HIT SOMETHING");
        if (occupied(current.type, current.x, current.y, current.dir)) {
          tetris_init();
        }
      }
      
    }
    
    
    function removeLines() {
   
      var x, y, complete, n = 0;
      	for(x = 0 ; x < nx ; x++) {
        complete = true;
        for(y = 0 ; y < ny ; y++) {
          if (!getBlock(x, y)) {
            complete = false;
            //console.log("still didnt complete line:" + x  + " - " + y);
          }
        }
        
        if (complete) {
          removeLine(x);
          x = x + 1; // recheck same line
          n++;
        }
      }
      
      if (n > 0) {
        //addRows(n);
        //addScore(100*Math.pow(2,n-1)); // 1: 100, 2: 200, 3: 400, 4: 800
      }
    
    }
    
    function removeLine(n) {
    	//console.log("YEAHH, REMOVE LINE" + n);
        var x, y;
        for(x = n ; x >= 0 ; --x) {
           for(y = 0 ; y < ny ; y++) {
           	setBlock(x, y, (x == 0) ? null : getBlock(x-1, y));
           }
        }
    }
    
    
    function dropPiece() {
      eachblock(current.type, current.x, current.y, current.dir, function(x, y) {
        setBlock(x, y, current.type);
      });
    }
    
    