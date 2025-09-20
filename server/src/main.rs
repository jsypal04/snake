use std::net::{SocketAddr, UdpSocket};
use serde::{Serialize, Deserialize};

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
struct GameState {
    apple_location: Coord,
    players: Vec<Snake>,
}

#[derive(Debug)]
struct Conn {
    snake_id: i32,
    addr: SocketAddr,
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

    fn connect(&mut self, addr: &SocketAddr, snake_id: &i32) {
        if !self.contains(addr) {
            // println!("Adding {} to connections table.", addr);
            let conn = Conn {
                snake_id: *snake_id,
                addr: *addr,
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

fn handle_packet(
    addr: SocketAddr, 
    packet: String, 
    connections: &mut Connections,
    state: &mut GameState,
) -> String {
    let mut data: Option<Snake> = None;
    let lines: Vec<&str> = packet.lines().collect();
    for (i, val) in lines.iter().enumerate() {
        if i == 0 {
            // println!("Authentication step.");
            continue;
        }
        let result = serde_json::from_slice(val.as_bytes());
        data = match result {
            Ok(val) => val,
            Err(err) => {
                println!("Error decoding packet: {err}, value: {val}");
                return String::from("404");
            }
        }
    }

    let snake_data = data.unwrap(); // TODO: Error handling here
    
    if !connections.contains(&addr) {
        // authenticate client
        println!("Connecting {addr}...");
        connections.connect(&addr, &snake_data.id);
        for con in connections.conns.iter() {
            println!("{con:#?}");
        }
    }

    connections.update_state(&snake_data.id, state, snake_data.coords);

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
        apple_location: apple_loc,
        players: players,
    }
} // --------------- end test function --------------------


fn main() {
    let socket = UdpSocket::bind("127.0.0.1:8000").unwrap();
    let mut connections = Connections {
        conns: Vec::new(),
    };
    let mut state = GameState {
        apple_location: Coord { x: 0.0, y: 0.0 },
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

