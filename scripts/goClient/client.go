package main

import (
  "encoding/binary"
  "math/rand"
  "fmt"
  "net"
)

const (
  SERVER_HOST = "localhost"
  SERVER_PORT = "8888"
  SERVER_TYPE = "tcp"
)

func main() {
  connection, err := net.Dial(SERVER_TYPE, SERVER_HOST + ":" + SERVER_PORT)
  if err != nil {
    panic(err)
  }
  buffer := make([]byte, 1024)
  mLen, err := connection.Read(buffer)

  if err != nil {
    fmt.Println("Error reading:", err.Error())
  }

  fmt.Println("Recieved: ", string(buffer[:mLen]))

  side := 'b'
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

  defer connection.Close()
}
