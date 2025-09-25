use std::net::{SocketAddr, UdpSocket};
use serde::{Serialize, Deserialize};
use rand::Rng;

// If a connection does not respond in 33msec (~double the sending frequency) it is removed from the connections table
const CONN_TIMEOUT: u128 = 30;

#[derive(Serialize, Deserialize, Debug)]
struct Coord {
    x: f32,
    y: f32,
}

#[derive(Serialize, Deserialize, Debug)]
struct Snake {
    id: i32,
    coords: Vec<Coord>, 
}

#[derive(Serialize, Deserialize, Debug)]
struct Apple {
    id: i32,
    location: Coord,
}

#[derive(Serialize, Deserialize, Debug)]
struct GameState {
    apple: Apple,
    players: Vec<Snake>,
}

#[derive(Debug)]
struct Conn {
    snake_id: i32,
    addr: SocketAddr,
    listener_port: i32,
    ttl: u128,
}

struct Connections {
    conns: Vec<Conn>,
}

impl Connections {
    fn contains(&self, src: &SocketAddr) -> bool {
        if self.conns.len() == 0 {
            return false;
        }

        for conn in &self.conns {
            if conn.addr == *src {
                return true;
            }
        }
        return true;
    } 

    fn push(&mut self, conn: Conn) {
        self.conns.push(conn);
    }

    fn connect(&mut self, addr: &SocketAddr, listener_port: i32, snake_id: &i32) {
        if !self.contains(addr) {
            // println!("Adding {} to connections table.", addr);
            let conn = Conn {
                snake_id: *snake_id,
                addr: *addr,
                listener_port: listener_port,
                ttl: CONN_TIMEOUT,
            };
            self.push(conn);
        }
    }

    fn disconnect(&mut self, addr: SocketAddr) {
        let result = self.conns.iter().position(|c| c.addr == addr);
        match result {
            None        => return,
            Some(index) => {
                self.conns.remove(index);
            }
        }
    }

    fn broadcast(&self, socket: &UdpSocket, state: &GameState) {
        let json_data = serde_json::to_string(state).unwrap(); // TODO: Error handling here
        // println!("{json_data}");
        
        // println!();
        for conn in self.conns.iter() {
            // println!("Broadcasting to {}", conn.addr);
            let _ = socket.send_to(json_data.as_bytes(), conn.addr); 
        }
    }

    fn update_ttls(&mut self) {
        let mut addrs_to_remove: Vec<SocketAddr> = vec![];
        for conn in self.conns.iter_mut() {
            let new_ttl = conn.ttl - 1;
            
            if new_ttl == 0 {
                addrs_to_remove.push(conn.addr);
            }
            else {
                conn.ttl = new_ttl;
            }
        }

        for addr in addrs_to_remove.iter() {
            println!("Disconnecting {addr}...");
            self.disconnect(*addr);
        }
    }

    fn update_state(
        &self, 
        snake_id: &i32,
        state: &mut GameState,
        new_coords: Vec<Coord>
    ) {
        if state.players.len() == 0 {
        }

        let mut index: Option<usize> = None;
        for (i, snake) in state.players.iter().enumerate() {
            if snake.id == *snake_id {
                index = Some(i);
            }
        }
        
        match index {
            Some(index) => state.players[index].coords = new_coords,
            None => {
                let player = Snake {
                    id: *snake_id,
                    coords: new_coords, 
                };
                state.players.push(player);
            },
        };
    }
}

/* fn get_now() -> u128 {
    let now = SystemTime::now();
    now
    .duration_since(UNIX_EPOCH)
    .expect("Time went backwards")
    .as_millis()
}*/

fn gen_apple_coords() -> Coord {
    let mut rng = rand::rng();

    Coord {
        x: rng.random_range(-1.0..=0.97),
        y: rng.random_range(-1.0..=0.97),
    }
}

fn handle_packet(
    addr: SocketAddr, 
    packet: String, 
    connections: &mut Connections,
    state: &mut GameState,
) -> String {
    let mut client_listener_port: i32;
    let mut data: Option<Snake> = None;
    let mut apple_collision_detected = false;

    let lines: Vec<&str> = packet.lines().collect();
    for (i, val) in lines.iter().enumerate() {
        if i == 0 {
            // println!("Authentication step.");
        } else if i == 1 {
            client_listener_port = match val.parse::<i32>() {
                Ok(port) => port,
                Err(err) => {
                    println!("Error extracting listener port {err}\n");
                    return String::from("402");
                }
            };

            
        } else if i == 2 {
            let result = serde_json::from_slice(val.as_bytes());
            data = match result {
                Ok(val) => val,
                Err(err) => {
                    println!("Error decoding packet: {err}, value: {val}");
                    return String::from("402");
                }
            };
        } else if i == 3 {
            apple_collision_detected = match *val {
                "Y" => true,
                _   => false,
            };
        }
    }

    let snake_data = data.unwrap(); // TODO: Error handling here
    
    if !connections.contains(&addr) {
        // authenticate client
        println!("Connecting {addr}...");
        connections.connect(&addr, client_listener_port, &snake_data.id);
        for con in connections.conns.iter() {
            println!("{con:#?}");
        }
    }

    connections.update_state(&snake_data.id, state, snake_data.coords);

    // Re-generate apple coords
    if apple_collision_detected {
        state.apple.id += 1;
        state.apple.location = gen_apple_coords();
    }

    // set ttl to CONN_TIMEOUT
    for conn in connections.conns.iter_mut() {
        if conn.snake_id == snake_data.id {
            conn.ttl = CONN_TIMEOUT;
            break;
        }
    }

    return String::from("200");
}

// ----------------- Test Function -----------------------
fn create_dummy_state() -> GameState {
    let apple_loc = Coord {
        x: 0.0,
        y: 0.0,
    };

    let mut players: Vec<Snake> = vec![];
    for i in 0..5 {
        let snake = Snake{
            id: i,
            coords: vec![Coord{ x: 1.0, y: 2.0 }],
        };
        players.push(snake);
    }

    GameState { 
        apple: Apple {
            id: 0,
            location: apple_loc,
        },
        players: players,
    }
} // --------------- end test function --------------------


fn main() {
    let socket = UdpSocket::bind("0.0.0.0:8000").unwrap();
    let mut connections = Connections {
        conns: Vec::new(),
    };
    let mut state = GameState {
        apple: Apple {
            id: 0,
            location: gen_apple_coords(),
        },
        players: vec![],
    };

    let mut buf = [0; 1024];
    let mut src: SocketAddr;
    let mut n: usize; 
    loop {
        // Receive packet
        (n, src) = socket.recv_from(&mut buf).expect("Error reading data.");
        // TODO: Error handling for the line below
        // println!("Received packet:\n{:#?}", String::from_utf8(buf[..n].to_vec()).unwrap());

        // handle packet
        let resp_str = handle_packet(
           src, 
           String::from_utf8(buf[0..n].to_vec()).unwrap(), // TODO: need to handle errors here.
           &mut connections,
           &mut state,
        );
        let response = resp_str.as_bytes();
        let _ = socket.send_to(response, src);

        // broadcast state if changed
        connections.broadcast(&socket, &state);

        // update connection ttls
        connections.update_ttls(); 

        // println!("x: {}, y: {}", state.players[0].coords[0].x, state.players[0].coords[0].y);
    }
}

