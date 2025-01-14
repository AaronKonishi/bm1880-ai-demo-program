#pragma once

#include <string>
#include <memory>
#include <vector>
#include <termios.h>
#include <mutex>
#include <map>
#include <condition_variable>
#include <queue>
#include "file_io.hpp"
#include "thread_runner.hpp"
#include "data_sink.hpp"
#include "data_source.hpp"
#include "packet.hpp"
#include "lru_cache.hpp"
#include "range.hpp"
#include "memory_view.hpp"

struct TtyConfig {
    struct termios settings() const;

    std::string device;
    int baud;
};

struct ReadSession {
    Memory::RangeSet pending;
    std::vector<unsigned char> data;
};

struct WriteSession {
    size_t total_size;
    Memory::RangeSet pending;
    std::vector<Memory::SharedView<unsigned char>> data;
};

class TtyIO : public PacketIO {
public:
    TtyIO(const TtyConfig &config);
    ~TtyIO();

    void close() override;
    bool is_opened() override;
    mp::optional<std::vector<Memory::SharedView<unsigned char>>> get() override;
    bool put(std::vector<Memory::SharedView<unsigned char>> &&obj) override;

private:
    void open(const TtyConfig &config);
    void open_pipe();
    void poll_io();
    void handle_read();
    void generate_output();
    void handle_write();
    bool read_header(PacketHeader &header);
    std::map<uint32_t, ReadSession>::iterator create_read_session(const PacketHeader &header);
    void handle_data(const PacketHeader &header);
    void handle_ack(const PacketHeader &header);
    void handle_request(const PacketHeader &header);
    void queue_output(std::lock_guard<std::mutex> &, uint32_t session_id, PacketHeader::PacketType type, uint32_t total_length, uint32_t data_offset, uint32_t data_length);
    void queue_output(uint32_t session_id, PacketHeader::PacketType type, uint32_t total_length, uint32_t data_offset, uint32_t data_length);
    std::vector<Memory::SharedView<unsigned char>> slice_segments(std::vector<unsigned char> &&data);

    std::atomic_bool closing_;

    std::mutex in_lock_;
    const uint32_t my_process_id;
    uint32_t peer_process_id = UINT32_MAX;
    std::vector<unsigned char> read_buffer_;
    std::map<uint32_t, ReadSession> read_sessions_;
    Memory::RangeSet pending_sessions;
    std::map<uint32_t, std::vector<unsigned char>> source_data_;

    std::mutex out_lock_;
    size_t next_write_session_id = 1;
    std::map<uint32_t, WriteSession> write_sessions_;
    std::queue<PacketHeader> output_queue_;

    size_t hangup_test_ = 0;
    FileIO file_;
    FileIO pipe_[2];
};
