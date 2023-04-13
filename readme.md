## Single-Producer Single-Consumer Queues

A playground to try and better understand concurrency models through single-producer single-consumer queues (SPSC).

Accompanying tests and benchmark harness provided.

**Queues implemented**
- Unbounded lockfree queue[^1]
- Circular buffer

**W.I.P**
- Bipartite Buffer


**Acknowledgements**
- [1024 Cores](https://www.1024cores.net/)
- [Cameron's awesome SPSC queue](https://moodycamel.com/blog/2013/a-fast-lock-free-queue-for-c++.htm)
- [Relacy Race Detector](https://github.com/dvyukov/relacy)



[^1]: [Simple, fast, and practical non-blocking and blocking concurrent queue algorithms](https://doi.org/10.1145/248052.248106)
