#pragma once
#include <string>
#include <vector>
#include <rote/rote.h>


namespace omnitty {


/**
 * @brief This class represents each machine the program interacts with
 */
class OmniMachine
{
public:
    /**
     * @brief Creates a new machine with the given name and virtual terminal dimensions.
     * @details This function starts a child ssh process to connect to the
     *          machine and puts its PID in the pid field of the returned machine
     *          structure. The ssh runs in the RoteTerm virtual terminal, whose
     *          address is also returned in the structure.
     * @param machineName Machine name.
     * @param vtRows Virtual terminal rows.
     * @param vtCols Virtual terminal Cols.
     */
    OmniMachine(const std::string &machineName, const std::string &command, int vtRows, int vtCols);


    /**
     * @brief Release the RoteTerm here.
     */
    ~OmniMachine();


    /**
     * @brief Is tagged.
     * @return Whether the machine tagged.
     */
    bool IsTagged() const { return m_isTagged; }


    /**
     * @brief Set tagged.
     * @param isTagged Set the machine tagged or not.
     */
    void SetIsTagged(bool isTagged) { m_isTagged = isTagged; }


    /**
     * @brief Toggle is tagged.
     */
    void ToggleIsTagged() { m_isTagged = !m_isTagged; }


    /**
     * @brief IsAlive
     * @return is alive
     */
    bool IsAlive() const { return m_isAlive; }


    /**
     * @brief SetIsAlive
     * @param isAlive whether alive
     */
    void SetIsAlive(bool isAlive) { m_isAlive = isAlive; }


    /**
     * @brief GetPid
     * @return the machine's pid
     */
    pid_t GetPid() const { return m_pid; }


    /**
     * @brief GetVirtualTerminal
     * @return the machine's terminal
     */
    RoteTerm *GetVirtualTerminal() const { return m_virtualTerminal; }


    /**
     * @brief GetMachineName
     * @return the machine's name
     */
    const std::string &GetMachineName() const { return m_machineName; }


    /**
     * @brief SetMachineName
     * @param machineName the name to set
     */
    void SetMachineName(const std::string &machineName) { m_machineName = machineName; }


    /**
     * @brief Save machine's 'tagged' state.
     */
    void PushMachineTag();


    /**
     * @brief Restore machine's 'tagged' state.
     */
    void PopMachineTag();


private:
    /** whether the machine is 'tagged' */
    bool                    m_isTagged;
    /** initially set to true; set to false when main program notifies that a
     *  certain pid has died and it matches this machine's ssh pid */
    bool                    m_isAlive;
    /** pid of ssh process running in terminal */
    pid_t                   m_pid;
    /** the machine's virtual terminal (ROTE library) */
    RoteTerm                *m_virtualTerminal;
    /** name of the machine */
    std::string             m_machineName;
    /** the following stack is used for storing the 'tagged' state for later retrieval */
    std::vector<uint8_t>    m_tagStack;
};


}

