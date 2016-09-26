package stats

import "github.com/beorn7/perks/quantile"

type Jitter struct {
	Histogram      *quantile.Stream
	rolling        float64
	nRollingJitter int
}

func NewJitter() *Jitter {
	return &Jitter{quantile.NewLowBiased(0.01), 0.0, 0}
}

func (j1 *Jitter) Combine(j2 *Jitter) {
	j1.nRollingJitter++
	j1.rolling += j2.rolling
}

func (j *Jitter) RollingJitter() float64 {
	n := 1
	if j.nRollingJitter > 0 {
		n = j.nRollingJitter
	}
	return j.rolling / float64(n)
}
func (j *Jitter) Insert(jit float64) {
	j.Histogram.Insert(jit)
	j.rolling += (jit - j.rolling) / 16.0
}
