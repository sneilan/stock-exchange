package main

import (
	"encoding/binary"
	"fmt"
	"io"
	"math/rand"
	"net"
  "time"
)

const (
  SERVER_HOST = "localhost"
  SERVER_PORT = "8888"
  SERVER_TYPE = "tcp"
)

func readIncomingTradeNotifications(conn net.Conn) {
  buffer := make([]byte, 4096)
  for {
    n, err := conn.Read(buffer)
    if err != nil {
      if err == io.EOF {
        fmt.Println("Exchange closed connection")
        return
      } else {
        fmt.Println("Could not read from socket")
        return
      }
    }
    fmt.Printf("Exchange said: %s\n", string(buffer[:n]))
  }
}

func main() {
  connection, err := net.Dial(SERVER_TYPE, SERVER_HOST + ":" + SERVER_PORT)
  if err != nil {
    panic(err)
  }

  go readIncomingTradeNotifications(connection)
  defer connection.Close()

  // buffer := make([]byte, 1024)
  // mLen, err := connection.Read(buffer)

  // if err != nil {
  //   fmt.Println("Error reading:", err.Error())
  // }

  // fmt.Println("Recieved: ", string(buffer[:mLen]))

  side := 'b'
  for {
    price := rand.Intn(900) + 100
    quantity := rand.Intn(10) * 10 + 100
    randomDigit := byte(0 + rand.Intn(10))

    writeBuf := make([]byte, 10)
    writeBuf[0] = byte(side)
    binary.LittleEndian.PutUint32(writeBuf[1:], uint32(price))
    binary.LittleEndian.PutUint32(writeBuf[4:], uint32(quantity))
    writeBuf[9] = randomDigit

    _, err = connection.Write(writeBuf)

    if err != nil {
      fmt.Println("Error writing", err.Error())
    }

    fmt.Println("Sent order", side, price, quantity)
    if side == 'b' {
      side = 's'
    } else {
      side = 'b'
    }

    time.Sleep(1 * time.Second)
  }
}
