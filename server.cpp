#include <grpcpp/grpcpp.h>
#include "mpi.grpc.pb.h"

using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::Status;
using mpi::MPIService;
using mpi::SendRequest;
using mpi::SendResponse;
using mpi::RecvRequest;
using mpi::RecvResponse;

class MPIServiceImpl final : public MPIService::Service {
  std::map<int32_t, std::string> messages_;

  Status Send(ServerContext* context, const SendRequest* request, SendResponse* response) override {
    messages_[request->receiver_rank()] = request->message();
    response->set_success(true);
    return Status::OK;
  }

  Status Recv(ServerContext* context, const RecvRequest* request, RecvResponse* response) override {
    auto it = messages_.find(request->receiver_rank());
    if (it != messages_.end()) {
      response->set_sender_rank(it->first);
      response->set_message(it->second);
      messages_.erase(it);
    } else {
      return Status(grpc::StatusCode::NOT_FOUND, "Message not found");
    }
    return Status::OK;
  }
};

void RunServer() {
  std::string server_address("0.0.0.0:50051");
  MPIServiceImpl service;

  ServerBuilder builder;
  builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
  builder.RegisterService(&service);

  std::unique_ptr<Server> server(builder.BuildAndStart());
  server->Wait();
}

int main(int argc, char** argv) {
  RunServer();
  return 0;
}
