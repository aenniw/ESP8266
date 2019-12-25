#ifndef ESP8266_PROJECTS_ROOT_SHARDSERVICE_H
#define ESP8266_PROJECTS_ROOT_SHARDSERVICE_H

#include <commons.h>
#include <WiFiUdp.h>
#include <service_log.h>
#include <set>

class ShardWorker : public Service {
public:
    virtual void start() = 0;

    virtual void stop() = 0;
};

class ShardService : public Service {
private:
    const uint8_t MAX_SCAN_SHARDS_FAILURES = 3;
    const char *SHARD_SLAVE = "slave_shard";
    const char *FIND_SHARD_SLAVE = "find_slave";
    const char *SHARD_MASTER = "master_shard";
    const char *FIND_SHARD_MASTER = "find_master";
    const uint16_t SHARD_PORT = 8492;
    const uint8_t MIN_SHARDS = 1;

    WiFiUDP *shardCoordinator = nullptr;

    std::set<IPAddress> slaveShardsIp;
    uint8_t scanSlaveShardsFailures = 0;
    bool scanningSlaveShards = false;

    std::set<IPAddress> masterShardsIp;

    uint8_t scanMasterShardsFailures = 0;
    bool scanningMasterShards = false;
    bool isMasterShard = false;

    uint64_t shard_loop = 0;

    std::set<ShardWorker *> workers;
protected:
    uint32_t get_freq() {
        return 30000;
    }

    bool connectedToNetwork();

    void registerSlaveInstance();

    void registerMasterInstance();

    void unregisterMasterInstance();

    bool isMasterCandidate();

    bool isConflictingMasterShard(const IPAddress &address);

    bool isShardCountOK();

    void sendMessage(const char *msg);

    void sendMessage(const char *msg, const IPAddress &address);

    void handleCoordination();

    void findMasterShards();

    void findSlaveShards();

public:
    ShardService();

    void registerWorker(ShardWorker *worker) {
        workers.insert(worker);
    }

    void cycle_routine() override;

};


#endif //ESP8266_PROJECTS_ROOT_SHARDSERVICE_H
