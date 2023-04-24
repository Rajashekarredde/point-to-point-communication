#include <iostream>
#include <memory>
#include <string>
#include <grpcpp/grpcpp.h>
#include "mpi.grpc.pb.h"

using grpc::Channel;
using grpc::ClientContext;
using grpc::Status;
using mpi::MPIService;
using mpi::SendRequest;
using mpi::SendResponse;
using mpi::RecvRequest;
using mpi::RecvResponse;

class MPIClient {
 public:
  MPIClient(std::shared_ptr<Channel> channel) : stub_(MPIService::NewStub(channel)) {}

  bool Send(int32_t sender_rank, int32_t receiver_rank, const std::string& message) {
    SendRequest request;
    request.set_sender_rank(sender_rank);
    request.set_receiver_rank(receiver_rank);
    request.set_message(message);

    SendResponse response;
    ClientContext context;
    Status status = stub_->Send(&context, request, &response);
    if (status.ok()) {
      return response.success();
    } else {
      std::cout << "Send RPC failed." << std::endl;
      return false;
    }
  }

  std::string Recv(int32_t receiver_rank, int32_t& sender_rank) {
    RecvRequest
    RecvRequest request;
    request.set_receiver_rank(receiver_rank);

    RecvResponse response;
    ClientContext context;
    Status status = stub_->Recv(&context, request, &response);
    if (status.ok()) {
      sender_rank = response.sender_rank();
      return response.message();
    } else {
      std::cout << "Recv RPC failed: " << status.error_message() << std::endl;
      return "";
    }
  }

 private:
  std::unique_ptr<MPIService::Stub> stub_;
};

int main(int argc, char** argv) {
  MPIClient client(grpc::CreateChannel("localhost:50051", grpc::InsecureChannelCredentials()));
  
  // Simulate an MPI_Send() operation
  int32_t sender_rank = 0;
  int32_t receiver_rank = 1;
  std::string message = "Hello from process 0!";
  bool send_success = client.Send(sender_rank, receiver_rank, message);
  if (send_success) {
    std::cout << "Message sent successfully." << std::endl;
  } else {
    std::cout << "Failed to send message." << std::endl;
  }

  // Simulate an MPI_Recv() operation
  int32_t received_sender_rank;
  std::string received_message = client.Recv(receiver_rank, received_sender_rank);
  if (!received_message.empty()) {
    std::cout << "Process " << receiver_rank << " received message: " << received_message
              << " from process " << received_sender_rank << std::endl;
  } else {
    std::cout << "Failed to receive message." << std::endl;
  }

  return 0;
}
