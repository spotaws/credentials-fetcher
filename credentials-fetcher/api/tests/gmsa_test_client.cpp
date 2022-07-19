#include <credentialsfetcher.grpc.pb.h>
#include <grpc++/grpc++.h>
#include <iostream>
#include <list>
#include <string>

#define unix_socket_address "unix:/usr/share/credentials-fetcher/socket/credentials_fetcher.sock"

/**
 * Testing client to validate grpc communication with server
 * Testing client to validate grpc communication with server, we need the client to mimic client
 * server communication behaviour
 * Invocation of the server can either be done from client or using grpc_cli
 *
 * Example cli invocations:
 * -------------------------
 * AddKerberoslease : grpc_cli {path_of_domain_sock}/credentials_fetcher.sock AddKerberoslease
 * "'cred_contents = {"webapp01$@CONTOSO.COM", "webapp02$@CONTOSO.COM"}'"
 * DeleteKerberoslease : grpc_cli {path_of_domain_sock}/credentials_fetcher.sock
 * "'DeleteKerberoslease "'lease_id = lease_id'"
 */
class CredentialsFetcherClient
{
  public:
    CredentialsFetcherClient( std::shared_ptr<grpc::Channel> channel )
        : _stub{ credentialsfetcher::CredentialsFetcherService::NewStub( channel ) }
    {
    }

    /**
     * Test method to create kerberos tickets
     * @param credspec_contents - information of service account
     * @return
     */

    std::string AddKerberosLeaseMethod( std::list<std::string> credspec_contents )
    {
        // Prepare request
        credentialsfetcher::CreateKerberosLeaseRequest request;
        for ( std::list<std::string>::const_iterator i = credspec_contents.begin();
              i != credspec_contents.end(); ++i )
        {
            request.add_credspec_contents( i->c_str() );
        }

        credentialsfetcher::CreateKerberosLeaseResponse response;
        grpc::ClientContext context;
        grpc::Status status;

        // Send request
        status = _stub->AddKerberosLease( &context, request, &response );

        // Handle response
        if ( status.ok() )
        {
            for ( int i = 0; i < response.created_kerberos_file_paths_size(); i++ )
            {
                std::cout << "created ticket file paths" + response.created_kerberos_file_paths( i )
                          << std::endl;
            }
            return response.lease_id();
        }
        else
        {
            std::cerr << status.error_code() << ": " << status.error_message() << std::endl;
            return "RPC failed";
        }
    }

    /**
     * Test method to delete kerberos tickets
     * @param credspec_contents - lease_id corresponding to the tickets created
     * @return
     */
    std::string DeleteKerberosLeaseMethod( std::string lease_id )
    {
        // Prepare request
        credentialsfetcher::DeleteKerberosLeaseRequest request;
        request.set_lease_id( lease_id );

        credentialsfetcher::DeleteKerberosLeaseResponse response;
        grpc::ClientContext context;
        grpc::Status status;

        // Send request
        status = _stub->DeleteKerberosLease( &context, request, &response );

        // Handle response
        if ( status.ok() )
        {
            for ( int i = 0; i < response.deleted_kerberos_file_paths_size(); i++ )
            {
                std::cout << "deleted ticket file paths" + response.deleted_kerberos_file_paths( i )
                          << std::endl;
            }
            return response.lease_id();
        }
        else
        {
            std::cerr << status.error_code() << ": " << status.error_message() << std::endl;
            return "RPC failed";
        }
    }

  private:
    std::unique_ptr<credentialsfetcher::CredentialsFetcherService::Stub> _stub;
};

int main( int argc, char** argv )
{
    std::string server_address{ unix_socket_address };
    CredentialsFetcherClient client{
        grpc::CreateChannel( server_address, grpc::InsecureChannelCredentials() ) };

    // create kerberos tickets
    std::list<std::string> credspec_contents = {
        "{\"CmsPlugins\":[\"ActiveDirectory\"],"
        "\"DomainJoinConfig\":{\"Sid\":\"S-1-5-21-4217655605-3681839426-3493040985\","
        "\"MachineAccountName\":\"WebApp01\",\"Guid\":\"af602f85-d754-4eea-9fa8-fd76810485f1\","
        "\"DnsTreeName\":\"contoso.com\",\"DnsName\":\"contoso.com\",\"NetBiosName\":\"contoso\"},"
        "\"ActiveDirectoryConfig\":{\"GroupManagedServiceAccounts\":[{\"Name\":\"WebApp01\","
        "\"Scope\":\"contoso.com\"},{\"Name\":\"WebApp01\",\"Scope\":\"contoso\"}]}}",
        "{\"CmsPlugins\":[\"ActiveDirectory\"],"
        "\"DomainJoinConfig\":{\"Sid\":\"S-1-5-21-4217655605-3681839426-3493040985\","
        "\"MachineAccountName\":\"WebApp03\",\"Guid\":\"af602f85-d754-4eea-9fa8-fd76810485f1\","
        "\"DnsTreeName\":\"contoso.com\",\"DnsName\":\"contoso.com\",\"NetBiosName\":\"contoso\"},"
        "\"ActiveDirectoryConfig\":{\"GroupManagedServiceAccounts\":[{\"Name\":\"WebApp03\","
        "\"Scope\":\"contoso.com\"},{\"Name\":\"WebApp03\",\"Scope\":\"contoso\"}]}}" };
    std::string add_response_field_lease_id = client.AddKerberosLeaseMethod( credspec_contents );
    std::cout << "Client received output for add kerberos lease: " << add_response_field_lease_id
              << std::endl;

    // delete kerberos tickets
    std::string lease_id = add_response_field_lease_id;
    std::string delete_response_field_lease_id = client.DeleteKerberosLeaseMethod( lease_id );
    std::cout << "Client received output for delete kerberos lease: "
              << delete_response_field_lease_id << std::endl;

    return 0;
}