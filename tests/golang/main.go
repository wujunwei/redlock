package main

import (
	"context"
	"fmt"
	"github.com/go-redis/redis/v8"
)

var ctx = context.Background()

func main() {
	rdb := redis.NewClient(&redis.Options{
		Addr:     "localhost:6379",
		Password: "", // no password set
		DB:       0,  // use default DB
	})
	defer func(rdb *redis.Client) {
		err := rdb.Close()
		if err != nil {
			fmt.Println(err)
		}
	}(rdb)
	//lock test
	fmt.Println(rdb.Do(ctx , "slock.lock","test",10000).String())
	//try unlock
	fmt.Println(rdb.Do(ctx, "slock.info","test").String())
	fmt.Println(rdb.Do(ctx, "slock.unlock","test").String())

}
