package stats

import (
	"fmt"
	"math/rand"

	"github.com/beorn7/perks/quantile"
)

type Stats struct {
	Jitter        *Jitter
	PktRecv       int64
	PktNum        int64
	BytesReceived int64
	Seconds       int
}

func NewStats() *Stats {
	return &Stats{
		NewJitter(), 0, 0, 0, 0,
	}
}

func (t *MCTest) CombineStats(s *Stats) {
	t.Stats.Combine(s)
}

func (s *Stats) Combine(s2 *Stats) {
	s.Jitter.Combine(s2.Jitter)
	s.PktRecv += s2.PktRecv
	s.PktNum += s2.PktNum
	s.BytesReceived += s2.BytesReceived
	s.Seconds += s2.Seconds
}

func TestQuantile() {
	s := quantile.NewHighBiased(0.01)
	for i := 0; i < 100000; i++ {
		s.Insert(rand.Float64())
	}
	for i := 1; i < 1000; i++ {
		f := float64(i) / 1000.0
		fmt.Printf("%d: %v\n", i, s.Query(f))
	}

}
