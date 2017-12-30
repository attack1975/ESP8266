var snake_playfield;
var total_rows = 8;
var total_colums = maxcolumns;

var score = 0;
var playfield;
var snake = new Array(3);
var snake_map;
var direction = 0;
var level = 0;

function snake_snakemove() {

}

function snake_snake_detectcoll() {

}

function snake_setdirection(dir) {
	if(snaketimer == null)
		return;

	if(direction == 0 && dir == 1) {
	
	} else if(direction == 1 && dir == 0) {
	
	} else if(direction == 2 && dir == 3) {
	
	} else if(direction == 3 && dir == 2) {
	
	} else {
		direction = dir;
	}
}
function snake_drawgame(map) 
{
    // Traverse all the body pieces of the snake, starting from the last one
    for (var i = snake.length - 1; i >= 0; i--) {
 
        // We're only going to perform the collision detection using the head
        // so it will be handled differently than the rest
        if (i === 0) {
            switch(direction) {
                case 0: // Right
                    snake[0] = { x: snake[0].x + 1, y: snake[0].y }
                    break;
                case 1: // Left
                    snake[0] = { x: snake[0].x - 1, y: snake[0].y }
                    break;
                case 2: // Up
                    snake[0] = { x: snake[0].x, y: snake[0].y - 1 }
                    break;
                case 3: // Down
                    snake[0] = { x: snake[0].x, y: snake[0].y + 1 }
                    break;
            }
 
            // Check that it's not out of bounds. If it is show the game over popup
            // and exit the function.
            if (snake[0].x < 0 || 
                snake[0].x >= total_colums ||
                snake[0].y < 0 ||
                snake[0].y >= 8) {
                snake_init();
                //showGameOver();
                return;
            }
 
            // Detect if we hit food and increase the score if we do,
            // generating a new food position in the process, and also
            // adding a new element to the snake array.
            if (map[snake[0].x][snake[0].y] === 1) {
                score += 10;
                map = snake_foodinit(map);
 
                // Add a new body piece to the array 
                snake.push({ x: snake[snake.length - 1].x, y: snake[snake.length - 1].y });
                map[snake[snake.length - 1].x][snake[snake.length - 1].y] = 2;
 
                // If the score is a multiplier of 100 (such as 100, 200, 300, etc.)
                // increase the level, which will make it go faster.
                if ((score % 100) == 0) {
                    level += 1;
                }
            
            // Let's also check that the head is not hitting other part of its body
            // if it does, we also need to end the game.
            } else if (map[snake[0].x][snake[0].y] === 2) {
                //showGameOver();
                snake_init();
                return;
            }
 
            map[snake[0].x][snake[0].y] = 2;
        } else {
            // Remember that when they move, the body pieces move to the place
            // where the previous piece used to be. If it's the last piece, it
            // also needs to clear the last position from the matrix
            if (i === (snake.length - 1)) {
                map[snake[i].x][snake[i].y] = null;
            }
 
            snake[i] = { x: snake[i - 1].x, y: snake[i - 1].y };
            map[snake[i].x][snake[i].y] = 2;
        }
    }
 
    // Draw the border as well as the score
    //drawMain();
}


function snake_frametimer() {
	//console.log("snake_frametimer");
	
	snake_drawgame(snake_map);
	
	var count = 0;
	 for(var x = 0; x < total_colums;  x++) {
	    	for(var y = 0; y < 8;  y++) {
	    		var clr = snake_map[x][y];
	    		
	    		if(clr == 1) {
		    		snake_playfield[count] = 0xFF;
					snake_playfield[count+1] = 0x00;
					snake_playfield[count+2] = 0x00;
				} else if(clr == 2) {
					snake_playfield[count] = 0xFF;
					snake_playfield[count+1] = 0xFF;
					snake_playfield[count+2] = 0xFF;
				} else {
					snake_playfield[count] = 0x00;
					snake_playfield[count+1] = 0x00;
					snake_playfield[count+2] = 0x00;
				}
				count+=3;
	    	}
	  }
	  
	send_leddata(snake_playfield);
	
	clearInterval(snaketimer);
	snaketimer = setInterval(snake_frametimer, 400 - (score) - (level*100) );
	
}

function snake_foodinit(map) {
// Generate a random position for the rows and the columns.
        var rndX = Math.round(Math.random() * total_colums-1),
            rndY = Math.round(Math.random() * 7);
        
        // We also need to watch so as to not place the food
        // on the a same matrix position occupied by a part of the
        // snake's body.
        while (map[rndX][rndY] > 0) {
            rndX = Math.round(Math.random() * total_colums-1);
            rndY = Math.round(Math.random() * 7);
        }
        
        map[rndX][rndY] = 1;
 
        
	return map;
}

function snake_snakeinit(map)
    {
        // Generate a random position for the row and the column of the head.
        var rndX = Math.round(Math.random() * 6),
            rndY = Math.round(Math.random() * 8);
 
        // Let's make sure that we're not out of bounds as we also need to make space to accomodate the
        // other two body pieces
        while ((rndX - snake.length) < 0) {
            rndX = Math.round(Math.random() * 6);
        }
        
        for (var i = 0; i < snake.length; i++) {
            snake[i] = { x: rndX - i, y: rndY };
            map[rndX - i][rndY] = 2;
        }
 
        return map;
    }
    

var snaketimer = null;

function snake_stop() {
	if(snaketimer != null) {
		clearInterval(snaketimer);
		snaketimer = null;
	}
}


function snake_init() 
{
	snake_playfield = new Array();
	total_colums = get_columns();
	
	var count = 0;
	 for(var x = 0; x < total_colums;  x++) {
	    	for(var y = 0; y < 8;  y++) {
	    		snake_playfield[count] = 0x00;
				snake_playfield[count+1] = 0x00;
				snake_playfield[count+2] = 0x00;
				count+=3;
	    	}
	  }
	
	snake = new Array(3);
	snake_map = new Array(total_colums);
    for (var i = 0; i < snake_map.length; i++) {
        snake_map[i] = new Array(8);
    }
    direction = 0;
	snake_map = snake_snakeinit(snake_map);
	
    // Add the food
    snake_map = snake_foodinit(snake_map);
    
	if(snaketimer != null)
		clearInterval(snaketimer);
	
	score = 0;
	level = 0;
	snaketimer = setInterval(snake_frametimer, 400 - (score) - (level*100) );	
}

function snake_start() {

}