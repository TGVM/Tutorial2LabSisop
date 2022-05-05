/*
 * SSTF IO Scheduler
 *
 * For Kernel 4.13.9
 */

#include <linux/blkdev.h>
#include <linux/elevator.h>
#include <linux/bio.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/init.h>

long long unsigned int curPos = 0;

/* SSTF data structure. */
struct sstf_data {
	struct list_head queue;
};

static void sstf_merged_requests(struct request_queue *q, struct request *rq,
				 struct request *next)
{
	list_del_init(&next->queuelist);
}

/* Esta função despacha o próximo bloco a ser lido. */
static int sstf_dispatch(struct request_queue *q, int force){
	struct sstf_data *nd = q->elevator->elevator_data;
	char direction = 'R';
	struct request *rq;
	rq = list_first_entry_or_null(&nd->queue, struct request, queuelist);

	/* Aqui deve-se retirar uma requisição da fila e enviá-la para processamento.
	 * Use como exemplo o driver noop-iosched.c. Veja como a requisição é tratada.
	 *
	 * Antes de retornar da função, imprima o sector que foi atendido.
	 */

	// Ponteiro para nodo
	struct list_head *ptr;
	// Itera sobre lista da fila
	list_for_each(ptr, &nd->queue) {
		printk(KERN_EMERG "entrou no for");

		rq = list_entry(ptr, struct request, queuelist);
		// Se posicao do req for maior q o ultimo
		if (blk_rq_pos(rq) > curPos) {
			printk(KERN_EMERG "entrou no if de escolha");

			//aux1 = abs(blk_rq_pos(rq) - curPos)
			//aux2  = abs(anterior -curPos)
			//if(aux1 < aux2) { escolhe aux1 } else aux2
			if(rq != list_first_entry_or_null(&nd->queue, struct request, queuelist)) {
				printk(KERN_EMERG "entrou no if de cabeca");

				long long unsigned int auxNext = abs(blk_rq_pos(rq) - curPos);
				long long unsigned int auxPrev = abs(blk_rq_pos(list_prev_entry(rq, queuelist)) - curPos);
				
				if(auxNext < auxPrev) {
					printk(KERN_EMERG "escolheu o next");

					break;
				} else {
					printk(KERN_EMERG "escolheu o prev");

					rq = list_prev_entry(rq, queuelist);
					break;
				}
			} else {
				printk(KERN_EMERG "escolheu a cabeca ou nulo");

				rq = list_first_entry_or_null(&nd->queue, struct request, queuelist);
				break;
			}
		}
	}

	//rq = list_first_entry_or_null(&nd->queue, struct request, queuelist);
	if (rq) {
		printk(KERN_EMERG "retorna o rq");

		curPos = blk_rq_pos(rq);

		printk(KERN_EMERG "linha 82 curPos = %llu", curPos);

		list_del_init(&rq->queuelist);

		printk(KERN_EMERG "foi dps linha 86");

		elv_dispatch_sort(q, rq);

		printk(KERN_EMERG "foi dps linha 90");

		printk(KERN_EMERG "[SSTF] dsp %c %llu\n", direction, blk_rq_pos(rq));

		return 1;
	}
	printk(KERN_EMERG "foi no fim");

	return 0;
}

static void sstf_add_request(struct request_queue *q, struct request *rq){
	struct sstf_data *nd = q->elevator->elevator_data;
	char direction = 'R';

	/* Aqui deve-se adicionar uma requisição na fila do driver.
	 * Use como exemplo o driver noop-iosched.c
	 *
	 * Antes de retornar da função, imprima o sector que foi adicionado na lista.
	 */

	// Ponteiro para nodo
	struct list_head *ptr;
	struct request *atual;
	if(list_empty(&nd->queue)) {
		printk(KERN_EMERG "ta vazio samerda");

		list_add_tail(&rq->queuelist, &nd->queue);
	} else {
		printk(KERN_EMERG "te coisa em samerda");

		// Itera sobre lista da fila
		list_for_each(ptr, &nd->queue) {
			atual = list_entry(ptr, struct request, queuelist);
			// Se posicao do req for maior q o ultimo
			if (blk_rq_pos(rq) > blk_rq_pos(atual)) {
				list_add_tail(&rq->queuelist, &atual->queuelist);
			}
		}
	}

	//list_add_tail(&rq->queuelist, &nd->queue);
	printk(KERN_EMERG "[SSTF] add %c %llu\n", direction, blk_rq_pos(rq));
}

static int sstf_init_queue(struct request_queue *q, struct elevator_type *e){
	struct sstf_data *nd;
	struct elevator_queue *eq;

	/* Implementação da inicialização da fila (queue).
	 *
	 * Use como exemplo a inicialização da fila no driver noop-iosched.c
	 *
	 */

	eq = elevator_alloc(q, e);
	if (!eq)
		return -ENOMEM;

	nd = kmalloc_node(sizeof(*nd), GFP_KERNEL, q->node);
	if (!nd) {
		kobject_put(&eq->kobj);
		return -ENOMEM;
	}
	eq->elevator_data = nd;

	INIT_LIST_HEAD(&nd->queue);

	spin_lock_irq(q->queue_lock);
	q->elevator = eq;
	spin_unlock_irq(q->queue_lock);

	return 0;
}

static void sstf_exit_queue(struct elevator_queue *e)
{
	struct sstf_data *nd = e->elevator_data;

	/* Implementação da finalização da fila (queue).
	 *
	 * Use como exemplo o driver noop-iosched.c
	 *
	 */
	BUG_ON(!list_empty(&nd->queue));
	kfree(nd);
}

/* Infrastrutura dos drivers de IO Scheduling. */
static struct elevator_type elevator_sstf = {
	.ops.sq = {
		.elevator_merge_req_fn		= sstf_merged_requests,
		.elevator_dispatch_fn		= sstf_dispatch,
		.elevator_add_req_fn		= sstf_add_request,
		.elevator_init_fn		= sstf_init_queue,
		.elevator_exit_fn		= sstf_exit_queue,
	},
	.elevator_name = "sstf",
	.elevator_owner = THIS_MODULE,
};

/* Inicialização do driver. */
static int __init sstf_init(void)
{
	return elv_register(&elevator_sstf);
}

/* Finalização do driver. */
static void __exit sstf_exit(void)
{
	elv_unregister(&elevator_sstf);
}

module_init(sstf_init);
module_exit(sstf_exit);

MODULE_AUTHOR("Miguel Xavier");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("SSTF IO scheduler");
