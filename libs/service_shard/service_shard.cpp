#include "service_shard.h"

void ShardService::registerSlaveInstance() {
    Log::println("Registered node to Slave shard.");
    isMasterShard = false;
}

void ShardService::registerMasterInstance() {
    Log::println("Registered node to Master shard.");
    isMasterShard = true;
    for (auto &worker:workers) {
        worker->start();
    }
}

void ShardService::unregisterMasterInstance() {
    if (isMasterShard) {
        Log::println("Unregistered node from Master shard.");
        for (auto &worker:workers) {
            worker->stop();
        }
    }
}

bool ShardService::isMasterCandidate() {
    if (!isShardCountOK()) {
        return false;
    }

    const uint32_t localIp = WiFi.localIP();
    for (auto &shardIp:slaveShardsIp) {
        const uint32_t remoteIp = shardIp;
        if (remoteIp >= localIp) {
            return false;
        }
    }
    return true;
}

bool ShardService::isConflictingMasterShard(const IPAddress &address) {
    return address >= WiFi.localIP();
}

bool ShardService::isShardCountOK() {
    return slaveShardsIp.size() >= MIN_SHARDS;
}

void ShardService::sendMessage(const char *msg) {
    if (!connectedToNetwork()) {
        return;
    }
    shardCoordinator->beginPacketMulticast(~WiFi.subnetMask() | WiFi.gatewayIP(), SHARD_PORT, WiFi.localIP());
    shardCoordinator->write(msg);
    shardCoordinator->endPacket();
}

void ShardService::sendMessage(const char *msg, const IPAddress &address) {
    if (!connectedToNetwork()) {
        return;
    }
    shardCoordinator->beginPacket(address, SHARD_PORT);
    shardCoordinator->write(msg);
    shardCoordinator->endPacket();
}

void ShardService::handleCoordination() {
    char buffer[128] = {'\0'};
    const auto address = shardCoordinator->remoteIP();
    shardCoordinator->readBytesUntil('\0', buffer, 128);

    if (strcmp(buffer, FIND_SHARD_SLAVE) == 0) {
        if (!isMasterShard)sendMessage(SHARD_SLAVE, shardCoordinator->remoteIP());
    } else if (strcmp(buffer, FIND_SHARD_MASTER) == 0) {
        if (isMasterShard)sendMessage(SHARD_MASTER, shardCoordinator->remoteIP());
    } else if (strcmp(buffer, SHARD_SLAVE) == 0) {
        slaveShardsIp.insert(address);
    } else if (strcmp(buffer, SHARD_MASTER) == 0) {
        masterShardsIp.insert(address);
    } else {
        Log::println("Unknown response %s", buffer);
    }
}

ShardService::ShardService() {
    shardCoordinator = new WiFiUDP();
    if (!shardCoordinator->begin(SHARD_PORT)) {
        Log::println("Error setting up udp responder!");
    }
    registerSlaveInstance();
}

void ShardService::findMasterShards() {
    scanningMasterShards = !scanningMasterShards;
    if (scanningMasterShards) {
        Log::println("Start scanning Master.");
        masterShardsIp.clear();
        sendMessage(FIND_SHARD_MASTER);
        return;
    }

    Log::println("End scanning Master.");
    const int n = masterShardsIp.size();
    if (isMasterShard) {
        for (auto &address:masterShardsIp) {
            yield();
            if (isConflictingMasterShard(address)) {
                unregisterMasterInstance();
                return;
            }
        }
    }
    if (!n) {
        scanMasterShardsFailures++;
        Log::println("No Master shards found. Errors: %d", scanMasterShardsFailures);
    } else {
        scanMasterShardsFailures = 0;
    }

    if (scanMasterShardsFailures >= MAX_SCAN_SHARDS_FAILURES) {
        scanMasterShardsFailures = 0;
        if (isMasterCandidate()) registerMasterInstance();
    }
}

void ShardService::findSlaveShards() {
    if (!isMasterShard && !masterShardsIp.empty()) {
        Log::println("Master shards up and ready, skipping Slave lookup.");
        return;
    }
    scanningSlaveShards = !scanningSlaveShards;
    if (scanningSlaveShards) {
        Log::println("Start scanning Slave.");
        slaveShardsIp.clear();
        sendMessage(FIND_SHARD_SLAVE);
        return;
    }

    Log::println("End scanning Slave.");
    const int n = slaveShardsIp.size();
    if (!n) {
        scanSlaveShardsFailures++;
        Log::println("No Slave shards found. Errors %d", scanSlaveShardsFailures);
    } else {
        scanSlaveShardsFailures = 0;
    }
    if (scanSlaveShardsFailures == MAX_SCAN_SHARDS_FAILURES) {
        scanSlaveShardsFailures = 0;
        slaveShardsIp.clear();
    }
}

void ShardService::cycle_routine() {
    if (isMasterShard) {
        for (auto &worker:workers) {
            yield();
            worker->cycle_routine();
        }
    }
    if (shardCoordinator->parsePacket()) {
        handleCoordination();
    } else {
        if (ESP.getCycleCount() % (3 * (get_freq())) == 0) {
            findSlaveShards();
        }
        if (ESP.getCycleCount() % (2 * (get_freq())) == 5000) {
            findMasterShards();
        }
    }
}

bool ShardService::connectedToNetwork() {
    return WiFi.status() == WL_CONNECTED;
}