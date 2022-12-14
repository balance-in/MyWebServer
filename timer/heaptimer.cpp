#include "heaptimer.h"

void HeapTimer::sift_up(size_t i) {
  assert(i >= 0 && i < heap_.size());
  size_t j = (i - 1) / 2;
  while (j >= 0) {
    if (heap_[j] < heap_[i]) break;
    swap_node(i, j);
    i = j;
    j = (i - 1) / 2;
  }
}

void HeapTimer::swap_node(size_t i, size_t j) {
  assert(i >= 0 && i < heap_.size());
  assert(j >= 0 && j < heap_.size());
  std::swap(heap_[i], heap_[j]);
  ref_[heap_[i].id] = i;
  ref_[heap_[j].id] = j;
}

bool HeapTimer::sift_down(size_t index, size_t n) {
  assert(index >= 0 && index < heap_.size());
  assert(n >= 0 && n <= heap_.size());
  size_t i = index;
  size_t j = i * 2 + 1;
  while (j < n) {
    if (j + 1 < n && heap_[j + 1] < heap_[j]) j++;
    if (heap_[i] < heap_[j]) break;
    swap_node(i, j);
    i = j;
    j = i * 2 + 1;
  }
  return i > index;
}

void HeapTimer::add(int id, int timeout, const TimerCallBack &cb) {
  assert(id >= 0);
  size_t i;
  if (ref_.count(id) == 0) {
    /* 新节点：堆尾插入，调整堆 */
    i = heap_.size();
    ref_[id] = i;
    heap_.push_back({id, Clock::now() + Ms(timeout), cb});
    sift_up(i);
  } else {
    /* 已有结点：调整堆 */
    i = ref_[id];
    heap_[i].expires = Clock::now() + Ms(timeout);
    heap_[i].time_cb = cb;
    if (!sift_down(i, heap_.size())) {
      sift_up(i);
    }
  }
}

void HeapTimer::do_work(int id) {
  /* 删除指定id结点，并触发回调函数 */
  if (heap_.empty() || ref_.count(id) == 0) {
    return;
  }
  size_t i = ref_[id];
  TimerNode node = heap_[i];
  node.time_cb();
  del_timer(i);
}

void HeapTimer::del_timer(size_t index) {
  /* 删除指定位置的结点 */
  assert(!heap_.empty() && index >= 0 && index < heap_.size());
  /* 将要删除的结点换到队尾，然后调整堆 */
  size_t i = index;
  size_t n = heap_.size() - 1;
  assert(i <= n);
  if (i < n) {  //不涉及到队尾元素
    swap_node(i, n);
    if (!sift_down(i, n)) {
      sift_up(i);
    }
  }
  /* 队尾元素删除 */
  ref_.erase(heap_.back().id);
  heap_.pop_back();
}

void HeapTimer::adjust(int id, int timeout) {
  /* 调整指定id的结点 */
  assert(!heap_.empty() && ref_.count(id) > 0);
  heap_[ref_[id]].expires = Clock::now() + Ms(timeout);
  sift_down(ref_[id], heap_.size());
}

void HeapTimer::tick() {
  /* 清除超时结点 */
  if (heap_.empty()) {
    return;
  }
  while (!heap_.empty()) {
    TimerNode node = heap_.front();
    if (std::chrono::duration_cast<Ms>(node.expires - Clock::now()).count() >
        0) {
      break;
    }
    node.time_cb();
    pop();
  }
}

void HeapTimer::pop() {
  assert(!heap_.empty());
  del_timer(0);
}

void HeapTimer::clear() {
  ref_.clear();
  heap_.clear();
}

int HeapTimer::get_next_tick() {
  tick();
  size_t res = -1;
  if (!heap_.empty()) {
    res = std::chrono::duration_cast<Ms>(heap_.front().expires - Clock::now())
              .count();
    if (res < 0) res = 0;
  }
  return res;
}