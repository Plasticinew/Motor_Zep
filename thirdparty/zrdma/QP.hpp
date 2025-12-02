#include "zQP.h"

namespace zrdma
{

enum ConnStatus {
  SUCC = 0,
  TIMEOUT = 1,
  WRONG_ARG = 2,
  ERR = 3,
  NOT_READY = 4,
  UNKNOWN = 5
};

constexpr struct timeval default_timeout = {0, 8000};
constexpr struct timeval no_timeout = {0, 0};  // it means forever

class RCQP{
    struct mr{
        uint64_t buf;
        uint32_t key;   
    };
public:
    RCQP(rkeyTable* rkey_table_, zTargetConfig& config, zEndpoint* ep_, zPD* pd_, string ip, string port, uint64_t local_addr, uint32_t lkey) {
        rpc_qp_ = zQP_create(pd_, ep_, rkey_table_, ZQP_RPC);
        zQP_connect(rpc_qp_, 0, ip, port);
        qp_ = zQP_create(pd_, ep_, rkey_table_, ZQP_ONESIDED);
        zQP_connect(qp_, 0, ip, port);
        uint64_t malloc_size = (size_t)1024*1024*1024*(6) + (size_t)1024*1024*2500;
        zQP_RPC_GetAddr(rpc_qp_, &remote_mr_.buf, &remote_mr_.key);
        local_mr_.buf = local_addr;
        local_mr_.key = lkey;
    }
    ~RCQP() = default;
    ConnStatus post_send(ibv_wr_opcode op, char* local_buf, uint32_t len, uint64_t off, int flags,
                       uint64_t wr_id = 0, uint32_t imm = 0) {
        ConnStatus ret = SUCC;
        struct ibv_send_wr* bad_sr;
            // setting the SGE
        struct ibv_sge sge{
        .addr = (uint64_t) local_buf,
        .length = len,
        .lkey = local_mr_.key
        };

        // setting sr, sr has to be initialized in this style
        struct ibv_send_wr sr;
        sr.wr_id = wr_id;
        sr.opcode = op;
        sr.num_sge = 1;
        sr.next = NULL;
        sr.sg_list = &sge;
        sr.send_flags = flags;
        sr.imm_data = imm;

        sr.wr.rdma.remote_addr = remote_mr_.buf + off;
        sr.wr.rdma.rkey = remote_mr_.key;
        std::vector<uint64_t> wr_ids;
        z_post_send_async(qp_, &sr, &bad_sr, true, 0, &wr_ids, true, -1);
        return SUCC;
    }

    ConnStatus post_batch(struct ibv_send_wr* send_sr, ibv_send_wr** bad_sr_addr, int num = 0) {
        std::vector<uint64_t> wr_ids;
        int rc = z_post_send_async(qp_, send_sr, bad_sr_addr, true, 0, &wr_ids, true, -1);
        return rc == 0 ? SUCC : ERR;
    }

    ConnStatus post_cas(char* local_buf, uint64_t off,
                      uint64_t compare, uint64_t swap, int flags, uint64_t wr_id = 0) {
        printf("CAS is not supported in RCQP!\n");
        return ERR;
    }

    ConnStatus poll_till_completion(ibv_wc& wc, struct timeval timeout = default_timeout) {
        int result = z_poll_till_completion(qp_);
        return result >= 0 ? SUCC : ERR;
    }

    int poll_send_completion(ibv_wc& wc) {
        return z_poll_send_completion(qp_, wc);
    }

    mr local_mr_;
    mr remote_mr_;
private:
    zQP* rpc_qp_{nullptr};
    zQP* qp_{nullptr};


};

}