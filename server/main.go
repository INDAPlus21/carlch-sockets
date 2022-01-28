package main

import (
       "bufio"
       "math/rand"
       "fmt"
       "net"
       "os"
       "strings"
       "time"
//       "container/list"
)

var m map[net.Conn]string

func firstWord(value string) string {
  for i := range value {
    if value[i] == ' ' {
      return value[0:i]
    } }
  return value }

func broadCast(c net.Conn, msg string) {
  for k := range m {
    if c != k {
      k.Write([]byte(msg))
    }
  } }

func getKey(val string, c net.Conn) net.Conn {
  for k := range m {
    if m[k] == val { return k }
  }
  return c
}

func handleConnection(c net.Conn) {
    fmt.Printf("Serving %s\n", c.RemoteAddr().String())

    netData, err    := bufio.NewReader(c).ReadString('\n')
    if err != nil {
      fmt.Println(err)
      return }
    userid          := strings.TrimSpace(string(netData))

    fmt.Printf("%s registered as %s\n", c.RemoteAddr().String(), userid)
    m[c] = userid

    for {
        // b               := ""
        netData, err    := bufio.NewReader(c).ReadString('\n')
        if err != nil {
           fmt.Println(err)
           return }
        temp            := strings.TrimSpace(string(netData))
        spliced         := strings.SplitN(temp, " ", 2)
        if spliced[0] == "SERVER" {
          broadCast(c, spliced[1])
          fmt.Println(spliced[1]);
          continue
        }
        ctemp           := getKey(spliced[0], c)
        if ctemp == c {
          c.Write([]byte("SERVER No such user is connected to server!"))
          continue
        }
        message         := spliced[1]
        ctemp.Write([]byte(message))
        c.Write([]byte(message))
    }
    delete(m, c)
    c.Close()
}

func main() {
  m = make(map[net.Conn]string)
  arguments := os.Args
  if len(arguments) == 1 {
    fmt.Println("Please provide a port number!")
    return
  }

  PORT := ":" + arguments[1]
  l, err := net.Listen("tcp4", PORT)
  if err != nil {
    fmt.Println(err)
    return
  }

  defer l.Close()
  rand.Seed(time.Now().Unix())

  for {
    c, err := l.Accept()
    if err != nil {
      fmt.Println(err)
      return
    }
    go handleConnection(c)
  }
}
