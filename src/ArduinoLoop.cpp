#include <Akka.h>

Mailbox* mailbox;
ActorContext* actorContext;

void loop() {
    mailbox = Mailbox::mailboxes().findFirst(
        [](Mailbox* mb) { return mb->hasMessages(); });
    if (mailbox)
        mailbox->handleMessages();
    actorContext =
        ActorContext::actorContexts().findFirst([](ActorContext* ac) {
            Timer* timer;
            if (ac->hasTimers() && (timer = ac->timers().findNextTimeout()) &&
                (timer->expiresAt() < Sys::millis())) {
                return true;
            };
            return false;
        });
    if (actorContext) {
        Envelope timerExpired(NoSender,actorContext->self(),TimerExpired);
        actorContext->self().tell(NoSender, timerExpired);
    };
}