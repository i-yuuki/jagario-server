#pragma once

constexpr size_t MAX_NAME_LENGTH = 64;
constexpr int PROTOCOL_VERSION = 1;

// 想定される送信元を名前の最初につけてるよ
// (クライアント→サーバー = C_HOGE)
enum class PacketType : short{
  C_JOIN,  // 参加リクエスト
  C_LEAVE, // 退出
  S_JOIN,  // 参加承認
};

struct PacketClientJoin{
  PacketType type = PacketType::C_JOIN;
  int version;
  char name[MAX_NAME_LENGTH];
};

struct PacketClientLeave{
  PacketType type = PacketType::C_LEAVE;
  unsigned int playerId;
};

struct PacketServerJoin{
  PacketType type = PacketType::S_JOIN;
  unsigned int playerId;
};
