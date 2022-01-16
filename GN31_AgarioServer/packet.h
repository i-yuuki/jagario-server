#pragma once

constexpr size_t MAX_NAME_LENGTH = 64;
constexpr int PROTOCOL_VERSION = 1;

// 想定される送信元を名前の最初につけてるよ
// (クライアント→サーバー = C_HOGE)
enum class PacketType : short{
  C_JOIN,          // 参加リクエスト
  C_LEAVE,         // 退出
  C_DIRECTION,     // 方向
  S_JOIN,          // 参加承認
  S_ADD_PLAYER,    // プレイヤーを追加
  S_REMOVE_PLAYER, // プレイヤーを削除 (死or退出)
  S_UPDATE_PLAYER, // プレイヤーの状態更新
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

struct PacketClientDirection{
  PacketType type = PacketType::C_DIRECTION;
  unsigned int playerId;
  float direction;
};

struct PacketServerJoin{
  PacketType type = PacketType::S_JOIN;
  unsigned int playerId;
};

struct PacketServerAddPlayer{
  PacketType type = PacketType::S_ADD_PLAYER;
  unsigned int playerId;
  float posX;
  float posY;
  float size;
  char name[MAX_NAME_LENGTH];
};

struct PacketServerRemovePlayer{
  PacketType type = PacketType::S_REMOVE_PLAYER;
  unsigned int playerId;
};

struct PacketServerUpdatePlayer{
  PacketType type = PacketType::S_UPDATE_PLAYER;
  unsigned int playerId;
  float posX;
  float posY;
  float size;
};
